#ifndef _SLD_PLATFORM_H_
#define _SLD_PLATFORM_H_

#include <new> // required for the placement new()

#include "SldMacros.h"
#include "SldTypeTraits.h"

/**
 * SLD__HAVE_SDC_BASE_TYPES_H
 *
 * If defined instructs the Engine to include `SDCBaseTypes.h` instead of
 * defining fundamental types itself. The header *must* define all of the
 * required fundamental types:
 *   Int[8|16|32|64]
 *   UInt[8|16|32|64]
 *   UInt4Ptr
 *   Float32
 *
 * The file should be somewhere on the include path
 *
 * NOTE: There is not much reason to use this unless the toolchain/platform
 *       does not provide <stdint.h>
 */
#ifdef SLD__HAVE_SDC_BASE_TYPES_H

#include "SDCBaseType.h"

#else

#include <stdint.h>

// Basic data types
typedef uint64_t  UInt64;
typedef int64_t   Int64;
typedef uint32_t  UInt32;
typedef int32_t   Int32;
typedef uint16_t  UInt16;
typedef int16_t   Int16;
typedef uint8_t   UInt8;
typedef int8_t    Int8;
typedef uintptr_t UInt4Ptr;

typedef float     Float32;

#endif // SLD__HAVE_SDC_BASE_TYPES_H

/**
 * SLD__HAVE_MEM_MGR
 *
 * If defined instructs the Engine to define all of the memory management
 * functions as exported (instead of static inlines) expecting them to be
 * present during linking.
 *
 * NOTE: Unless you need to use a custom allocator there is no point using
 *       this
 *
 * There are also 2 defines for more fine-grained control of this:
 *  SLD__HAVE_MEM_MGR_ALLOCATOR
 *    controls memory allocation/deallocation functions (malloc()/free() & friends)
 *  Sld__HAVE_MEM_MGR_MISC
 *    controls miscellaneous memory functions (memset()/memcpy() & friends)
 * So it's eg. possible to use a custom memory allocator and still use library
 * memcpy()/memset().
 */
#ifdef SLD__HAVE_MEM_MGR
#  define SLD__HAVE_MEM_MGR_ALLOCATOR
#  define SLD__HAVE_MEM_MGR_MISC
#endif

#ifdef SLD__HAVE_MEM_MGR_ALLOCATOR

// Allocates memory aSize bytes
void *sldMemNew(UInt32 aSize);

// Frees up memory
void sldMemFree(void *aPointer);

// Reassigns memory aSize bytes
void *sldMemRealloc(void *aPointer, UInt32 aSize);

// Allocates memory aSize bytes filled with zeros
void *sldMemNewZero(UInt32 aSize);

#else

#include <stdlib.h> /* malloc, calloc, free */

/**
 * Allocates a block of memory
 *
 * @param [in] aSize - the size of the memory block in bytes
 *
 * @return a pointer to a block of memory, or NULL on error
 */
static inline void *sldMemNew(UInt32 aSize)
{
	return malloc(aSize);
}

/**
 * Allocates memory full of 0
 *
 * @param[in] aSize - memory block size in bytes
 *
 * @return a pointer to a block of memory, or NULL on error
 ************************************************************************/
static inline void *sldMemNewZero(UInt32 aSize)
{
	return calloc(1, aSize);
}

/**
 * Resizes the allocated block of memory
 *
 * @param aPtr  - pointer to block
 * @param aSize - memory block size in bytes
 *
 * @return a pointer to a block of memory, or NULL on error
 */
static inline void *sldMemRealloc(void *aPtr, UInt32 aSize)
{
	return realloc(aPtr, aSize);
}

/**
 * Free memory allocated using the #SLDMEMEMEW, #Sldmemerewzero, #SldMemrelloc
 *
 * @param[in] aPtr - pointer to memory to be freed
 */
static inline void sldMemFree(void *aPtr)
{
	free(aPtr);
}

#endif // SLD__HAVE_MEM_MGR_ALLOCATOR

#ifdef SLD__HAVE_MEM_MGR_MISC

// Copies memory
void sldMemMove(void *aToPtr, const void *aFromPtr, UInt32 aSize);

// Copies memory
void sldMemCopy(void *aToPtr, const void *aFromPtr, UInt32 aSize);

// Fills memory with aValue values
void sldMemSet(void *aPtr, Int32 aValue, UInt32 aSize);

#else

#include <string.h> /* memset, memmove, memcpy */

/**
 * Copies memory from one location to another
 *
 * @param[out] aToPtr   - pointer to memory block where data from aFromPtr should be placed
 * @param[in]  aFromPtr - pointer to the memory block from where to get the data for copying into aToPtr
 * @param[in]  aSize    - amount of data to copy
 */
static inline void sldMemMove(void *aToPtr, const void *const aFromPtr, UInt32 aSize)
{
	memmove(aToPtr, aFromPtr, aSize);
}

/**
 * Copies memory from one location to another
 *
 * @param[out] aToPtr   - pointer to memory block where data from aFromPtr should be placed
 * @param[in]  aFromPtr - pointer to the memory block from where to get the data for copying into aToPtr
 * @param[in]  aSize    - amount of data to copy
 *
 * IMPORTANT: memory blocks must not overlap
 */
static inline void sldMemCopy(void *aToPtr, const void *const aFromPtr, UInt32 aSize)
{
	memcpy(aToPtr, aFromPtr, aSize);
}

/**
 * Fills memory with values
 *
 * @param[in] aPtr   - pointer to the block of memory to be filled
 * @param[in] aValue - the value to fill the memory block
 * @param[in] aSize  - block size to be filled
 */
static inline void sldMemSet(void *aPtr, Int32 aValue, UInt32 aSize)
{
	memset(aPtr, aValue, aSize);
}

#endif // SLD__HAVE_MEM_MGR_MISC

/**
 * Cleans up memory (fills 0)
 *
 * @param[in] aPtr  - pointer to the block of memory to be cleared
 * @param[in] aSize - block size to be cleaned
 */
static inline void sldMemZero(void *aPtr, UInt32 aSize)
{
	sldMemSet(aPtr, 0, aSize);
}

/**
 * Allocates memory for object (s) of type @T
 *
 * @param[in] aCount - number of objects (> 0, by default - 1)
 *
 * @return pointer to object (s) or NULL on error
 */
template <typename T>
static inline T* sldMemNew(UInt32 aCount = 1)
{
	return aCount > 0 ? (T*)sldMemNew(sizeof(T) * aCount) : NULL;
}

/**
 * Allocates memory for object (s) of type @T filled with 0
 *
 * @param[in] aCount - number of objects (> 0, by default - 1)
 *
 * @return pointer to object (s) or NULL on error
 */
template <typename T>
static inline T* sldMemNewZero(UInt32 aCount = 1)
{
	return aCount > 0 ? (T*)sldMemNewZero(sizeof(T) * aCount) : NULL;
}

/**
 * Resizes the allocated memory block for an array of objects of type @T
 *
 * @param[in] aPtr   - pointer to an array of objects
 * @param[in] aCount - number of objects
 *
 * @return pointer to an array of objects, or NULL on error
 */
template <typename T>
static inline T* sldMemReallocT(T *aPtr, UInt32 aCount)
{
	return (T*)sldMemRealloc(aPtr, sizeof(T) * aCount);
}

// custom implementation of std::move, std::forward and the supporting meta machinery
namespace sld2 {

// std :: move implementation
template<class T>
inline remove_reference<T>&& move(T&& aArg)
{
	return static_cast<remove_reference<T>&&>(aArg);
}

// implementation of std :: forward
template<class T>
inline T&& forward(remove_reference<T>& aArg)
{
	return static_cast<T&&>(aArg);
}

template<class T>
inline T&& forward(remove_reference<T>&& aArg)
{
	static_assert(!is_lvalue_reference<T>::value, "Can not forward an rvalue as an lvalue.");
	return static_cast<T&&>(aArg);
}

template <typename T, typename... Args>
inline void construct_at(T* aPtr, Args&&... aArgs)
{
	sld2_assume(aPtr != nullptr);
	::new (static_cast<void*>(aPtr)) T(forward<Args>(aArgs)...);
}

template <typename T, enable_if<!is_trivially_destructible<T>::value> = 0>
inline void destroy_at(T *aPtr) { aPtr->~T(); }
template <typename T, enable_if<is_trivially_destructible<T>::value> = 0>
inline void destroy_at(T*) { /* noop */ }

} // namespace sld2

/**
 * Creates an object of type @T passing @aArgs arguments to the constructor
 *
 * Use instead of pure new (), delete via sldDelete ()
 *
 * @return pointer to newly created object, or NULL if memory cannot be allocated
 */
template<typename T, typename... Args>
inline T *sldNew(Args&&... aArgs)
{
	T *p = sldMemNew<T>();
	if (p)
		sld2::construct_at(p, sld2::forward<Args>(aArgs)...);
	return p;
}

/**
 * Destroys the object created with sldNew ()
 *
 * @param[in] aPtr - pointer to object
 */
template <typename T>
inline void sldDelete(T *aPtr)
{
	sld2::destroy_at(aPtr);
	sldMemFree(aPtr);
}

#endif //_SLD_PLATFORM_H_
