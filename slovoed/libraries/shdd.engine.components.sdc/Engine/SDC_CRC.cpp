#include "SDC_CRC.h"

/*
Calculates the checksum of a data block.

The following can be used as the base polynomial
numbers: 0x04C11DB7 / 0xEDB88320 / 0x82608EDB
The most commonly used is 0xEDB88320.

@param buf - pointer to a buffer with data for calculating CRC32
@param size - data size for CRC32 calculation
@param start_value - the initial value (the result of the previous
calling this function). The first call should be 0xFFFFFFFFUL.
@param inverse_flag - flag whether to invert bits in crc at the end of calculation or not
*/
UInt32 CRC32(const UInt8 *buf, UInt32 size, UInt32 start_value, bool inverse_flag)
{
	UInt32 crc_table[256];
	UInt32 crc;

	for (UInt32 i = 0; i < 256; i++)
	{
		crc = i;
		for (UInt32 j = 0; j < 8; j++)
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

		crc_table[i] = crc;
	}

	crc = start_value;

	while (size)
	{
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
		size--;
	}
	
	if (inverse_flag)
		crc ^= 0xFFFFFFFFUL;

	return crc;
}
