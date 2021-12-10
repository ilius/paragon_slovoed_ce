#ifndef _SDC_H_
#define _SDC_H_

#include "SldPlatform.h"

/*

	@page Container of dictionaries.

	Dictionaries contain heterogeneous data, which, for ease of storage,
	distribution and processing should be combined into a single file which serves
	container and provides search and access to the data you need.

	Previously, the container was the PRC format (Palm Resource Code: http://en.wikipedia.org/wiki/PRC_(Palm_OS))
	However, he had some problems that prevent him from using it in the future:
	1) Limiting the resource size to 64kB
	2) The number of resources is limited - no more than 65565 per type
	3) Structures are not aligned by 4 bytes, which is why additional actions are required when reading
	4) The need to flip data during operation - not Intel (ARM, MIPS) byte order in a machine word.
	5) There is no control over the integrity of the file (even the file size is not present).

	Therefore, starting in 2009, the transition to a new container begins, which we will call
	SDC (SlovoEd Data Container).

	All separated data will be called resources. Resources can be of type (Huffman tree,
	comparison table, compressed data, dictionary header, etc.) and number (halfman tree 0,
	halfman tree 1, ..., halfman tree 8). The resource number can be any and not necessary
	in a row with other resources of the same type. There can be many different types of resources.
	There can be many resources of different numbers. There can be many resources with the same number
	or type, but there cannot be resources having the same type and number among themselves.
	The container structure will be similar to the PRC structure:
								\n
	[SDC Header]				\n
								\n
	[Resource record 0]			\n
	......						\n
	[Resource record N]			\n
								\n
	[Data]						\n

	The SDC Header contains the header of the container that stores general information.
	Resource record X are records with information about where resources are located.
	Data - the actual data that should be stored in the container.
	Attention! The actual resource data goes in the same order as the records in the Resource record.
*/

// File header structure
typedef struct SlovoEdContainerHeader
{
	// File signature, must be "SLD2"
	UInt32	Signature;
	// The size of the header structure.
	UInt32	HeaderSize;
	// Container version.
	UInt32	Version;
	// Checksum for the file.
	UInt32	CRC;
	// File size.
	UInt32	FileSize;
	// Container id
	UInt32	DictID;
	// The number of resources in the file.
	UInt32	NumberOfResources;
	// The size of the #SlovoEdContainerResourcePosition structure.
	/** needed in case we need to create a dictionary larger than 4GB */
	UInt32	ResourceRecordSize;
	// Container content type (see #ESlovoEdContainerDatabaseTypeEnum).
	UInt32	DatabaseType;
	// If the flag is not equal to 0, then the records in the resource table are sorted:
	// by resource type, and within a range of one type also by resource number
	UInt32	IsResourceTableSorted;
	// Number of additional base properties
	UInt32 BaseAddPropertyCount;
	// Flag that this is a demo database
	UInt32 IsInApp;
	// Flag, if not equal to 0, then there are resource names
	UInt8	IsResourcesHaveNames;
	// If the flag is not equal to 0, then there are compressed resources in the database
	UInt8	HasCompressedResources;
	// explicit alignment to next UInt32
	UInt16	_pad0;
	// Reserved
	UInt32	Reserved[19];
}SlovoEdContainerHeader;

// A structure describing the location of the resource.
typedef struct SlovoEdContainerResourcePosition
{
	// Resource type
	UInt32 Type;
	// Resource index
	UInt32 Index;
	// Resource size
	// The most significant bit is used as a flag of resource compression, respectively, the current
	// the maximum size of one resource is limited to 2GB
	UInt32 Size;
	// Offset from the beginning of the file to the beginning of the resource
	UInt32 Shift;
}SlovoEdContainerResourcePosition;

// The type of algorithm by which the resource is compressed
enum ESDCResourceCompressionType {
	// Without compression
	eSDCResourceCompression_None = 0
};

// The structure describing the compressed resource; stored * before * the compressed resource data
struct SlovoEdContainerCompressedResourceHeader
{
	// The type of algorithm by which the resource is compressed (see #ESDCResourceCompressionType)
	UInt16 CompressionType;
	// Align to next UInt32 (theoretically can be used for customization
	// header for different compression algorithms)
	UInt16 _pad0;
	// Resource size in uncompressed, original form
	UInt32 UncompressedSize;
};

// Line size with additional dictionary properties
#define DEFAULT_PROPERTY_SIZE		256

// A structure describing additional properties of the base
struct TBaseProperty
{
	// Property name
	UInt16 PropertyName[DEFAULT_PROPERTY_SIZE];

	// Property Description
	UInt16 Property[DEFAULT_PROPERTY_SIZE];
};

// The current version number of the container.
#define SDC_CURRENT_VERSION		(0x00000101)
// Container signature - SLD2
#define SDC_SIGNATURE			('2DLS')

// Enumeration of errors in working with the container.
enum SDCError
{
	// No mistakes.
	SDC_OK = 0,

	// Base for memory errors.
	SDC_MEM_ERRORS = 0x0100,
	// Null pointer passed
	SDC_MEM_NULL_POINTER,
	// Out of memory
	SDC_MEM_NOT_ENOUGH_MEMORY,

	// Writing class errors
	SDC_WRITE_ERRORS = 0x0200,
	// Tried to add an empty resource.
	SDC_WRITE_EMPTY_RESOURCE,
	// Such a resource already exists.
	SDC_WRITE_ALREADY_EXIST,
	// Error creating file (opening for writing)
	SDC_WRITE_CANT_CREATE_FILE,
	// File write error.
	SDC_WRITE_CANT_WRITE,
	// Resource table sorting error
	SDC_WRITE_CANT_SORT_RESOURCE_TABLE,

	// Errors of the class responsible for reading.
	SDC_READ_ERRORS = 0x0300,
	// File opening error.
	SDC_READ_CANT_OPEN_FILE,
	// I cannot read the specified amount of data.
	SDC_READ_CANT_READ,
	// Incorrect signature, then the file is not in SDC format!
	SDC_READ_WRONG_SIGNATURE,
	// Invalid resource number
	SDC_READ_WRONG_INDEX,
	// It is not possible to position according to the data from the offset table.
	SDC_READ_CANT_POSITIONING,
	// The resource can not be found
	SDC_READ_RESOURCE_NOT_FOUND,
	// Container not open
	SDC_READ_NOT_OPENED,
	// Incorrect file size
	SDC_READ_WRONG_FILESIZE,
	// Checksum error
	SDC_READ_WRONG_CRC,
	// Invalid property index
	SDC_READ_WRONG_PROPERTY_INDEX

};

// The structure describing the data of the read resource
typedef struct ResourceMemType
{
	// Pointer to data
	void	*ptr;

	// Data size
	UInt32	Size;

	// Resource type
	UInt32	Type;

	// Resource number
	UInt32	Index;
}ResourceMemType;

#endif // _SDC_H_
