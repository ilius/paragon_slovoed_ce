#ifndef _SLD_SDC_READ_MY_H_
#define _SLD_SDC_READ_MY_H_

#include "SldPlatform.h"
#include "SldError.h"
#include "SldTypes.h"
#include "SldDefines.h"
#include "SldDynArray.h"
#include "ISDCFile.h"

// simple_list
namespace sld2 {

// a kernel/bsd style intrusive list node
struct list_node
{
	list_node *next;
	list_node *prev;

	list_node();
};

// "initializes" the list to an "empty" state
static inline void make_empty(list_node *node) {
	node->next = node;
	node->prev = node;
}

// removes the list_node from the parent list
static inline void unlink(list_node *node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	make_empty(node);
}

inline list_node::list_node() {
	make_empty(this);
}

// the list head (different type to enforce type safety a bit)
struct list_head : public list_node {};

} // namespace sld2

// A class for reading data from a container.
class CSDCReadMy
{
	struct ResourceStruct
	{
		// refcount
		int refcnt;

		// data size
		UInt32 size;
		// type
		UInt32 type;
		// index among resources of the same type
		UInt32 index;
		// resource data
		void *data;

		// backref to the reader
		CSDCReadMy &reader;
		// list handling
		sld2::list_node link;

		ResourceStruct(CSDCReadMy &reader_);
		~ResourceStruct();

		void clear();

		void ref() { refcnt++; }
		ResourceStruct* unref()
		{
			refcnt--;
			if (refcnt > 0)
				return this;
			reader.CloseResource(this);
			return nullptr;
		}
	};

public:
	// The class represents the "handle" of the loaded resource
	class Resource
	{
	public:
		Resource() : ptr_(nullptr) {}
		Resource(const Resource &h) : ptr_(h.ptr_) { ref(); }
		Resource(Resource&& h) : ptr_(h.ptr_) { h.ptr_ = nullptr; }

		~Resource() { unref(); }

		Resource& operator=(const Resource &h)
		{
			unref();
			ptr_ = h.ptr_;
			ref();
			return *this;
		}

		Resource& operator=(Resource&& h)
		{
			unref();
			ptr_ = h.ptr_;
			h.ptr_ = nullptr;
			return *this;
		}

		// checks if this handle is "pointing" to a valid opened resource
		bool empty() const { return ptr_ == nullptr; }

		// returns a pointer to the resource data
		const UInt8* ptr() const { return empty() ? nullptr : (const UInt8*)ptr_->data; }

		// returns the size of the resource data
		UInt32 size() const { return empty() ? 0 : ptr_->size; }

		// returns resource type
		UInt32 type() const { return empty() ? 0 : ptr_->type; }

		// returns resource index (among the resources of the same type)
		UInt32 index() const { return empty() ? 0 : ptr_->index; }

		// returns resource data in the form of a span
		sld2::Span<const UInt8> data() const {
			return empty() ? nullptr : sld2::make_span((const UInt8*)ptr_->data, ptr_->size);
		}

		// helper to easily see if we have a resource loaded
		explicit operator bool() const { return !empty(); }

	protected:
		explicit Resource(ResourceStruct *ptr) : ptr_(ptr) {}

		void ref() { if (ptr_) ptr_->ref(); }
		void unref() { if (ptr_) ptr_ = ptr_->unref(); }

	private:
		ResourceStruct *ptr_;
	};

	// The class represents the "handle" of the *just* loaded resource
	// In fact, Resource + loading status (if eOK - meaning the resource has been loaded successfully)
	class ResourceHandle : protected Resource
	{
		friend class CSDCReadMy;
	public:
		ResourceHandle() : error_(eMemoryNullPointer) {}

		// returns the underlying loading status code
		ESldError error() const { return error_; }

		// helper to see if we successfuly loaded the resource
		explicit operator bool() const { return error_ == eOK; }

		// returns the underlying resource handle
		const Resource& resource() const { return *this; }

		// checks if this handle is "pointing" to a successfully loaded resource
		bool empty() const { return error_ != eOK || Resource::empty(); }

		// accessors to the underlying resource
		using Resource::ptr;
		using Resource::size;
		using Resource::type;
		using Resource::index;
		using Resource::data;

	private:
		ResourceHandle(ESldError error) : error_(error) {}
		explicit ResourceHandle(ResourceStruct *ptr) : Resource(ptr), error_(eOK) {}

		ESldError error_;
	};

public:
	// Constructor
	CSDCReadMy(void);

	// Destructor
	~CSDCReadMy(void);

	// Opens the container
	ESldError Open(ISDCFile *aFile);

	// Closes the container.
	void Close();

	// Get a resource by its type and number
	ResourceHandle GetResource(UInt32 aResType, UInt32 aResIndex);

	// Get resource data by its type and number without memory allocation
	ESldError GetResourceData(void* aData, UInt32 aResType, UInt32 aResIndex, UInt32 *aDataSize);

	// Get resource data by its type and number without memory allocation
	ESldError GetResourceData(void* aData, UInt32 aResType, UInt32 aResIndex, UInt32 aDataSize)
	{
		return GetResourceData(aData, aResType, aResIndex, &aDataSize);
	}

	// Gets the offset from the beginning of the file to the resource with the given type and number
	ESldError GetResourceShiftAndSize(UInt32 *aShift, UInt32 *aSize, UInt32 aResType, UInt32 aResIndex) const;

	// Returns the base property for the given key
	bool GetPropertyByKey(const UInt16* aKey, UInt16** aValue);
	// Returns the number of additional properties of the base
	UInt32 GetNumberOfProperty() const;
	// Returns the key and value of the base property by the property index
	SDCError GetPropertyByIndex(UInt32 aPropertyIndex, UInt16** aKey, UInt16** aValue);

	// Returns the CRC of the file for the given header and output stream
	static SDCError GetFileCRC(const SlovoEdContainerHeader *aHeader, ISDCFile* aFileData, UInt32* aFileCRC);

	// Returns the type of content in the container
	UInt32 GetDatabaseType(void) const;

	// Checks whether the database is complete or not
	UInt32 IsInApp(void) const { return m_Header.IsInApp; }

	// We check the integrity of the container.
	SDCError CheckData(void);

	// Gets a pointer to the current container file
	ISDCFile* GetFileData();

private:

	// Returns the number of resources in an open container.
	UInt32 GetNumberOfResources() const;

	// Gets the index of a resource in the resource location table by its type and number
	UInt32 GetResourceIndexInTable(UInt32 aResType, UInt32 aResIndex) const;

	// Releases resource data if its refcount falls to 0
	void CloseResource(ResourceStruct *aResource);

	static inline ResourceStruct* to_resource(sld2::list_node *node) {
		return sld2_container_of(node, ResourceStruct, &ResourceStruct::link);
	}

private:

	// Open container file
	ISDCFile								*m_FileData;

	// Container header
	SlovoEdContainerHeader					m_Header;

	// Resource Location Table
	SlovoEdContainerResourcePosition*		m_resTable;

	// List of loaded active (used) resources
	sld2::list_head							m_loadedResources;

	// List of inactive resources
	sld2::list_head							m_freeList;

	// Buffer for storing the current property
	TBaseProperty*							m_Property;

	// Cache for reading packed resources
	sld2::DynArray<UInt8>					m_compressedData;
};

// ResourceHandle equality comparsion operators to simplify error checking
inline bool operator==(const CSDCReadMy::ResourceHandle &res, ESldError error) { return res.error() == error; }
inline bool operator!=(const CSDCReadMy::ResourceHandle &res, ESldError error) { return res.error() != error; }

#endif
