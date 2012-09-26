/*
	NAND_DEVICE.H
	-------------
*/
#ifndef NAND_DEVICE_H_
#define NAND_DEVICE_H_

#include <stdint.h>

/*
	class ATOSE_NAND_DEVICE
	-----------------------
*/
class ATOSE_nand_device
{
public:
	/*
		Timing parameters
	*/
	uint32_t frequency;			// the frequency (in MHz) the NAND chip runs at (default = 10Mhz)
	uint32_t address_setup;		// in nanoseconds;
	uint32_t data_hold;			// in nanoseconds;
	uint32_t data_setup;		// in nanoseconds;

	/*
		Physical parameters
	*/
	uint32_t bytes_per_sector;				// in bytes; the number of bytes per block
	uint32_t metadata_bytes_per_sector;		// in bytes; number of metadata bytes per block
	uint32_t sectors_per_block;				// sectors per block
	uint32_t ecc_level;						// preferred ECC level
} ;


#endif /* NAND_DEVICE_H_ */
