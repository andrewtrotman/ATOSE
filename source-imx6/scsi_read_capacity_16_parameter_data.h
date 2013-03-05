/*
	SCSI_READ_CAPACITY_16_PARAMETER_DATA.H
	--------------------------------------
*/

#ifndef SCSI_READ_CAPACITY_16_PARAMETER_DATA_H_
#define SCSI_READ_CAPACITY_16_PARAMETER_DATA_H_

#include "msb_lsb.h"

/*
	class ATOSE_SCSI_READ_CAPACITY_16_PARAMETER_DATA
	------------------------------------------------
*/
class ATOSE_scsi_read_capacity_16_parameter_data
{
public:
	ATOSE_msb_uint64_t returned_logical_block_address;
	ATOSE_msb_uint32_t block_length_in_bytes;
	union
		{
		uint32_t all;
		struct
			{
			unsigned lowest_aligned_logical_block_address : 14;			// WARNING:: this is MSB...LSB and therefore you can't just read it.
			unsigned tprz : 1;
			unsigned tpe : 1;
			unsigned logical_blocks_per_physical_block_exponent : 4;
			unsigned p_i_exponent : 4;
			unsigned prot_en      : 1;
			unsigned p_type       : 3;
			unsigned reserved     : 4;
			} ;
		} ;
	uint8_t reserved2[16];
} ;

#endif
