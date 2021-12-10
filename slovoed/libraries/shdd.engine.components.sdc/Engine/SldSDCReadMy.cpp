#include "SldSDCReadMy.h"

#include "SDC_CRC.h"
#include "SldCompare.h"
#include "SldDynArray.h"

// The size of one data block in bytes for calculating the CRC of the direct resource data
#define CRC_DATA_BLOCK_SIZE			(0xFFFF)

enum : UInt32 { InvalidResourceIndex = ~0u };

// TODO: Add an explicit flag to the header?
static inline bool hasCompression(const SlovoEdContainerHeader &aHeader)
{
	return aHeader.HasCompressedResources != 0;
}

// resource record compression accessor
static inline bool isCompressed(const SlovoEdContainerResourcePosition &aResource)
{
	return (aResource.Size & (1u << 31)) != 0;
}

// list manipulation helpers
namespace {

static bool empty(const sld2::list_head &list) {
	return list.next == &list;
}

static void push_front(sld2::list_head &list, sld2::list_node *node) {
	node->prev = &list;
	node->next = list.next;
	list.next->prev = node;
	list.next = node;
}

static void move_to_front(sld2::list_head &list, sld2::list_node *node) {
	unlink(node);
	push_front(list, node);
}

static sld2::list_node* pop_front(sld2::list_head &list) {
	sld2::list_node *node = list.next;
	unlink(node);
	return node;
}

} // anon namespace

// resource loading
// the main reason for all this template mumbo-jumbo is to let
// the compiler optimize the relevant "fast paths"
namespace {

namespace readers {

struct File {
	ISDCFile *file;
	UInt32 offset;
	File(ISDCFile *file, UInt32 offset) : file(file), offset(offset) {}
	bool operator()(void *aData, UInt32 aSize) const
	{
		return file->Read(aData, aSize, offset) == aSize;
	}
};

// resource loader that always allocates storage for a resource
template <typename Reader>
static ESldError allocate(void **aData, UInt32 aReadSize, Reader&& aReader)
{
	*aData = sldMemNew(aReadSize);
	if (*aData == NULL)
		return eMemoryNotEnoughMemory;

	if (aReader(*aData, aReadSize))
		return eOK;

	sldMemFree(*aData);
	*aData = NULL;
	return eResourceCantGetResource;
}

// resource loader that tries to load the resource into the supplied buffer
template <typename Reader>
static ESldError inplace(void *aData, UInt32 aSize, UInt32 aReadSize, Reader&& aReader)
{
	if (aSize >= aReadSize)
		return aReader(aData, aReadSize) ? eOK : eResourceCantGetResource;

	// slowpath for when the passed in buffer is too small
	void *data;
	ESldError error = allocate(&data, aReadSize, sld2::forward<Reader>(aReader));
	if (error == eOK)
	{
		sldMemCopy(aData, data, aSize);
		sldMemFree(data);
	}
	return error;
}

} // namespace readers

// multiplexer over the different loader impls
template <bool InPlace, typename Reader>
static inline ESldError read(void **aData, UInt32 *aSize, UInt32 aReadSize, Reader&& aReader)
{
	ESldError error;
	if (InPlace)
		error = readers::inplace(*aData, *aSize, aReadSize, sld2::forward<Reader>(aReader));
	else
		error = readers::allocate(aData, aReadSize, sld2::forward<Reader>(aReader));

	if (error == eOK)
		*aSize = aReadSize;
	return error;
}

template <bool InPlace>
static inline ESldError loadCompressed(const SlovoEdContainerResourcePosition &aResource,
									   ISDCFile *aFile, sld2::DynArray<UInt8> &aCompressedData,
									   void **aData, UInt32 *aSize)
{
	// assert(isCompressed(aResource));
	// as the highest bit is used as a flag we have to mask it off
	UInt32 size = aResource.Size & ~(1u << 31);

	if (size > aCompressedData.size())
	{
		if (!aCompressedData.resize(sld2::default_init, size))
			return eMemoryNotEnoughMemory;
	}

	if (aFile->Read(aCompressedData.data(), size, aResource.Shift) != size)
		return eResourceCantGetResource;

	auto header = (const SlovoEdContainerCompressedResourceHeader*)aCompressedData.data();
	const char *data = (const char*)aCompressedData.data() + sizeof(*header);
	size -= sizeof(*header);

	return eResourceCantGetResource;
}

} // anon namespace

CSDCReadMy::ResourceStruct::ResourceStruct(CSDCReadMy &reader_)
	: refcnt(0), size(0), type(0), index(0), data(nullptr), reader(reader_)
{}

CSDCReadMy::ResourceStruct::~ResourceStruct()
{
	clear();
	unlink(&link);
}

void CSDCReadMy::ResourceStruct::clear()
{
	refcnt = 0;
	if (data)
		sldMemFree(data);
	data = nullptr;
	size = type = index = 0;
}

CSDCReadMy::CSDCReadMy(void)
{
	sldMemZero(&m_Header, sizeof(m_Header));
	m_resTable = NULL;
	m_Property = NULL;
	m_FileData = NULL;
}

CSDCReadMy::~CSDCReadMy(void)
{
	Close();

	while (!empty(m_freeList))
		sldDelete(to_resource(pop_front(m_freeList)));
}

/**
 * Opens a container file
 * 
 * @param[in] aFileName - open sdc container file
 * 
 * @return error code
 */
ESldError CSDCReadMy::Open(ISDCFile *aFile)
{
	if (!aFile)
		return eMemoryNullPointer;
	if (!aFile->IsOpened())
		return eResourceCantOpenContainer;

	Close();

	m_FileData = aFile;

	if (m_FileData->Read(&m_Header, sizeof(m_Header), 0) != sizeof(m_Header))
	{
		Close();
		return eResourceCantOpenContainer;
	}

	if (m_Header.Signature != SDC_SIGNATURE)
	{
		Close();
		return eResourceCantOpenContainer;
	}

	if (m_Header.HeaderSize > sizeof(m_Header) ||
		m_Header.Version > SDC_CURRENT_VERSION ||
		m_Header.ResourceRecordSize != sizeof(SlovoEdContainerResourcePosition))
	{
		Close();
		return eCommonTooHighDictionaryVersion;
	}

	m_resTable = sldMemNew<SlovoEdContainerResourcePosition>(m_Header.NumberOfResources);
	if (!m_resTable)
	{
		Close();
		return eMemoryNotEnoughMemory;
	}

	const UInt32 resTableSize = sizeof(m_resTable[0]) * m_Header.NumberOfResources;
	if (m_FileData->Read(m_resTable, resTableSize, m_Header.HeaderSize) != resTableSize)
	{
		Close();
		return eResourceCantOpenContainer;
	}

	if (m_Property)
		sldMemZero(m_Property, sizeof(*m_Property));

	return eOK;
}

/***********************************************************************
* Closes the file and frees memory.
************************************************************************/
void CSDCReadMy::Close()
{
	m_FileData = NULL;

	if (m_resTable)
		sldMemFree(m_resTable);
	m_resTable = NULL;

	while (!empty(m_loadedResources))
	{
		ResourceStruct *resource = to_resource(pop_front(m_loadedResources));
		resource->clear();
		push_front(m_freeList, &resource->link);
	}

	if (m_Property)
		sldMemFree(m_Property);
	m_Property = NULL;

	sldMemZero(&m_Header, sizeof(m_Header));
}

/***********************************************************************
* Returns the type of content in the container
*
* @return the type of content in the container
************************************************************************/
UInt32 CSDCReadMy::GetDatabaseType(void) const
{
	if (m_FileData && m_FileData->IsOpened())
		return m_Header.DatabaseType;

	return eDatabaseType_Unknown;
}

/***********************************************************************
* Returns the number of resources in the container.
*
* @return the number of resources or 0 (if the container is not open).
************************************************************************/
UInt32 CSDCReadMy::GetNumberOfResources() const
{
	if (m_FileData && m_FileData->IsOpened())
		return m_Header.NumberOfResources;

	return 0;
}

/***********************************************************************
* Checks if the data is valid in the container, if there is any corruption.
*
* @return error code
************************************************************************/
SDCError CSDCReadMy::CheckData(void)
{
	if (!m_FileData)
		return SDC_MEM_NULL_POINTER;
	if (!m_FileData->IsOpened())
		return SDC_READ_NOT_OPENED;

	if (m_FileData->GetSize() != m_Header.FileSize)
		return SDC_READ_WRONG_FILESIZE;

	// We save the CRC and set it to zero in the structure,
	// since calculation of CRC32 when creating a container
	// happened with m_Header.CRC = 0
	const UInt32 CRC = m_Header.CRC;
	m_Header.CRC = 0;

	UInt32 new_CRC;
	SDCError error = GetFileCRC(&m_Header, m_FileData, &new_CRC);

	// Recovering the original CRC
	m_Header.CRC = CRC;

	if (error != SDC_OK)
		return error;

	return new_CRC == m_Header.CRC ? SDC_OK : SDC_READ_WRONG_CRC;
}

/***********************************************************************
* Returns the number of additional properties of the base
*
* @return number of additional base properties
************************************************************************/
UInt32 CSDCReadMy::GetNumberOfProperty() const
{
	return m_Header.BaseAddPropertyCount;
}

/***********************************************************************
* Returns the base property for the given key
* Memory for aValue is allocated inside the reader class and cleared by itself when it is closed
*
* @param[in]	aKey	- required key
* @param[out]	aValue	- the pointer to which the base property will be written
*
* @return search result by key:		false	- key not found
									true	- key found
************************************************************************/
bool CSDCReadMy::GetPropertyByKey(const UInt16* aKey, UInt16** aValue)
{
	if (!m_FileData || m_Header.BaseAddPropertyCount == 0)
		return false;

	if (m_Property == NULL)
	{
		m_Property = sldMemNew<TBaseProperty>();
		if (m_Property == NULL)
			return false;
	}

	UInt32 propertyPos = m_Header.FileSize - (m_Header.BaseAddPropertyCount * sizeof(TBaseProperty));

	Int32 upperbound = m_Header.BaseAddPropertyCount;
	Int32 lowerbound = 0;
	while (lowerbound <= upperbound)
	{
		const Int32 med = (upperbound + lowerbound) >> 1;

		const UInt32 offset = propertyPos + med * sizeof(TBaseProperty);
		m_FileData->Read(m_Property->PropertyName, sizeof(m_Property->PropertyName), offset);

		const Int32 cmp = CSldCompare::StrCmp(m_Property->PropertyName, aKey);
		if (cmp == 0)
		{
			m_FileData->Read(m_Property->Property, sizeof(m_Property->Property),
							 offset + sizeof(m_Property->PropertyName));
			*aValue = m_Property->Property;
			return true;
		}
		else if (cmp < 0)
		{
			lowerbound = med + 1;
		}
		else
		{
			upperbound = med - 1;
		}
	}

	sldMemZero(m_Property, sizeof(*m_Property));
	return false;
}

/***********************************************************************
* Returns the key and value of the base property by the property index
* Memory for aValue and aKey is allocated inside the reader class and cleared by itself when it is closed
*
* @param[in]	aPropertyIndex	- requested property index
* @param[out]	aKey			- pointer to which the key of the base property will be written
* @param[out]	aValue			- the pointer to which the base property will be written
*
* @return error code
************************************************************************/
SDCError CSDCReadMy::GetPropertyByIndex(UInt32 aPropertyIndex, UInt16** aKey, UInt16** aValue)
{
	if (!m_FileData)
		return SDC_MEM_NULL_POINTER;
	if(aPropertyIndex >= m_Header.BaseAddPropertyCount)
		return SDC_READ_WRONG_PROPERTY_INDEX;

	if (m_Property == NULL)
	{
		m_Property = sldMemNew<TBaseProperty>();
		if (m_Property == NULL)
			return SDC_MEM_NOT_ENOUGH_MEMORY;
	}

	UInt32 propertyPos = m_Header.FileSize - ((aPropertyIndex + 1) * sizeof(TBaseProperty));
	m_FileData->Read(m_Property, sizeof(*m_Property), propertyPos);

	*aKey = m_Property->PropertyName;
	*aValue = m_Property->Property;
	return SDC_OK;
}

/***********************************************************************
* Method for getting data from container
*
* @param[in] aResType	- resource type
* @param[in] aResIndex	- resource number for the specified type.
*
* @return handle of the loaded resource
************************************************************************/
CSDCReadMy::ResourceHandle CSDCReadMy::GetResource(UInt32 aResType, UInt32 aResIndex)
{
	if (!m_FileData)
		return eResourceCantGetResource;

	// check if the resource is already loaded
	for (sld2::list_node *node = m_loadedResources.next; node != &m_loadedResources; node = node->next)
	{
		ResourceStruct *resource = to_resource(node);
		if (resource->type == aResType && resource->index == aResIndex)
		{
			resource->ref();
			move_to_front(m_loadedResources, node);
			return ResourceHandle(resource);
		}
	}

	// looking for a global resource index
	UInt32 index = GetResourceIndexInTable(aResType, aResIndex);
	if (index == InvalidResourceIndex)
		return eResourceCantGetResource;

	const SlovoEdContainerResourcePosition &position = m_resTable[index];

	// Reading the resource
	void *ptr;
	UInt32 size;
	ESldError err;
	if (hasCompression(m_Header) && isCompressed(position))
	{
		err = loadCompressed<false>(position, m_FileData, m_compressedData, &ptr, &size);
	}
	else
	{
		err = read<false>(&ptr, &size, position.Size, readers::File(m_FileData, position.Shift));
	}
	if (err != eOK)
		return err;

	ResourceStruct *resource;
	if (!empty(m_freeList))
		resource = to_resource(pop_front(m_freeList));
	else
		resource = sldNew<ResourceStruct>(*this);
	if (!resource)
	{
		sldMemFree(ptr);
		return eMemoryNotEnoughMemory;
	}

	resource->refcnt = 1;
	resource->data = ptr;
	resource->size = size;
	resource->index = position.Index;
	resource->type = position.Type;

	push_front(m_loadedResources, &resource->link);
	return ResourceHandle(resource);
}

/**
 * Frees resource data
 */
void CSDCReadMy::CloseResource(ResourceStruct *aResource)
{
	// assert(aResource);
	// assert(aResource->refcnt <= 0);
	move_to_front(m_freeList, &aResource->link);
	aResource->clear();
}

/**
 * We get resource data by its type and number without memory allocation
 *
 * @param[out]   aData     - pointer to the allocated area of memory, by which
 *                           data from the resource will be written
 * @param[in]    aResType  - resource type
 * @param[in]    aResIndex - resource number for the specified type
 * @param[inout] aDataSize - pointer to the size of the allocated memory area,
 *                           the *real* size of the read data will also be written here
 *
 * @return error code
 */
ESldError CSDCReadMy::GetResourceData(void* aData, UInt32 aResType, UInt32 aResIndex, UInt32 *aDataSize)
{
	if (!m_FileData)
		return eResourceCantGetResource;
	if (!aData || !aDataSize)
		return eMemoryNullPointer;
	if (*aDataSize == 0)
		return eOK;

	// looking for a global resource index
	UInt32 index = GetResourceIndexInTable(aResType, aResIndex);
	if (index == InvalidResourceIndex)
		return eResourceCantGetResource;

	const SlovoEdContainerResourcePosition &resource = m_resTable[index];
	if (hasCompression(m_Header) && isCompressed(resource))
		return loadCompressed<true>(resource, m_FileData, m_compressedData, &aData, aDataSize);

	const UInt32 readSize = (sld2::min)(resource.Size, *aDataSize);
	if (m_FileData->Read(aData, readSize, resource.Shift) != readSize)
		return eResourceCantGetResource;

	*aDataSize = readSize;
	return eOK;
}

/***********************************************************************
* Returns the CRC of the file for the given header and output stream
*
* @param[in]	aHeader		- pointer to the file header for which you want to
                              calculate the checksum
* @param[in]	aFileData	- pointer to the stream for which the checksum
                              needs to be calculated
* @param[out]	aFileCRC	- a pointer to where the CRC of the file will be written
*
* @return error code
************************************************************************/
SDCError CSDCReadMy::GetFileCRC(const SlovoEdContainerHeader *aHeader, ISDCFile* aFileData, UInt32* aFileCRC)
{
	if (!aHeader || !aFileData)
		return SDC_MEM_NULL_POINTER;

	// CRC container header
	UInt32 new_CRC = CRC32((const UInt8*)aHeader, sizeof(*aHeader), SDC_CRC32_START_VALUE, true);

	sld2::DynArray<UInt8> buf;

	// Resource table CRC
	const UInt32 resTableSize = aHeader->ResourceRecordSize * aHeader->NumberOfResources;
	if (!buf.resize(sld2::default_init, resTableSize))
		return SDC_MEM_NOT_ENOUGH_MEMORY;

	if (aFileData->Read(buf.data(), resTableSize, aHeader->HeaderSize) != resTableSize)
		return SDC_READ_CANT_READ;

	new_CRC = CRC32(buf.data(), resTableSize, new_CRC, true);

	if (!buf.resize(sld2::default_init, CRC_DATA_BLOCK_SIZE))
		return SDC_MEM_NOT_ENOUGH_MEMORY;

	// Calculate the CRC sequentially for each data block
	UInt32 offset = aHeader->HeaderSize + resTableSize;
	UInt32 data_size = aHeader->FileSize - offset;
	while (data_size)
	{
		// The size of the next data block for reading
		UInt32 readSize = (data_size > CRC_DATA_BLOCK_SIZE) ? CRC_DATA_BLOCK_SIZE : data_size;

		if (aFileData->Read(buf.data(), readSize, offset) != readSize)
			return SDC_READ_CANT_READ;

		data_size -= readSize;
		offset += readSize;

		new_CRC = CRC32(buf.data(), readSize, new_CRC, data_size == 0);
	}

	*aFileCRC = new_CRC;
	return SDC_OK;
}

/***********************************************************************
* Gets the offset from the beginning of the file to the resource with the given type and number
*
* @param[out] aShift	- pointer to the variable to which the shift will be stored
* @param[in] aResType	- resource type
* @param[in] aResIndex	- resource number for the specified type.
*
* @return error code
************************************************************************/
ESldError CSDCReadMy::GetResourceShiftAndSize(UInt32 *aShift, UInt32 *aSize, UInt32 aResType, UInt32 aResIndex) const
{
	if (!aShift)
		return eMemoryNullPointer;

	*aShift = -1;

	UInt32 indexInTable = GetResourceIndexInTable(aResType, aResIndex);
	if (indexInTable == InvalidResourceIndex)
		return eResourceCantGetResource;

	*aShift = m_resTable[indexInTable].Shift;
	*aSize = m_resTable[indexInTable].Size;

	return eOK;
}

/***********************************************************************
* Get resource index in the resource location table by its type and number
*
* @param[in] aResType		- resource type
* @param[in] aResIndex		- resource number for the specified type.
*
* @return resource index in the resource location table or InvalidResourceIndex if there is no such resource
************************************************************************/
UInt32 CSDCReadMy::GetResourceIndexInTable(UInt32 aResType, UInt32 aResIndex) const
{
	// We are looking for among all available resources
	UInt32 ResourceCount = GetNumberOfResources();

	// Binary search
	if (m_Header.IsResourceTableSorted)
	{
		if (m_resTable[0].Type > aResType || m_resTable[ResourceCount-1].Type < aResType)
			return InvalidResourceIndex;

		struct pred {
			UInt32 type, index;
			bool operator()(const SlovoEdContainerResourcePosition &pos) const {
				return pos.Type == type ? pos.Index < index : pos.Type < type;
			}
		};

		UInt32 index = sld2::lower_bound(m_resTable, ResourceCount, pred{ aResType, aResIndex });
		if (index == ResourceCount)
			return InvalidResourceIndex;

		if (m_resTable[index].Type == aResType && m_resTable[index].Index == aResIndex)
			return index;
	}
	// Linear search
	else
	{
		for (UInt32 i=0;i<ResourceCount;i++)
		{
			if (m_resTable[i].Type == aResType && m_resTable[i].Index == aResIndex)
				return i;
		}
	}

	return InvalidResourceIndex;
}

/***********************************************************************
* Gets an open container file
*
* @return pointer to the object to read data from the container
************************************************************************/
ISDCFile* CSDCReadMy::GetFileData()
{
	return m_FileData;
}
