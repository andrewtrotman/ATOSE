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
	uint32_t block_size;		// in bytes; the number of bytes per block
	uint32_t metadata_size;		// in bytes; number of metadata bytes per block
	uint32_t blocks_per_page;	// blocks per page
	uint32_t spare_per_page;	// in bytes; the number of "spate" bytes per page (for ECC)
	uint32_t ecc_level;			// 
} ;


#endif /* NAND_DEVICE_H_ */
