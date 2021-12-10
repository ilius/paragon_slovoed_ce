#pragma once
#ifndef _I_SDC_FILE_H_
#define _I_SDC_FILE_H_

#include "SDC.h"

class ISDCFile
{
public:
	// Destructor
	virtual ~ISDCFile(void) {}

	// Checks if a file is open. 1 - if open, 0 if not
	virtual Int8 IsOpened() const = 0;

	/**
	 * Reads a block of data from a file
	 *
	 * @param[in] aDestPtr - pointer where the read data will be written
	 * @param[in] aSize    - data block size (in bytes)
	 * @param[in] aOffset  - offset (in bytes) relative to the beginning of the file from where read
	 *
	 * @return size of the read data block (in bytes)
	 */
	virtual UInt32 Read(void *aDestPtr, UInt32 aSize, UInt32 aOffset) = 0;

	// Returns the size of the file in bytes
	virtual UInt32 GetSize() const = 0;
};

#endif // _I_SDC_FILE_H_
