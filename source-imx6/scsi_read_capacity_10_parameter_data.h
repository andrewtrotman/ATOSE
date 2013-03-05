/*
	SCSI_READ_CAPACITY_10_PARAMETER_DATA.H
	--------------------------------------
*/

#ifndef SCSI_READ_CAPACITY_10_PARAMETER_DATA_H_
#define SCSI_READ_CAPACITY_10_PARAMETER_DATA_H_

#include "msb_lsb.h"

/*
	class ATOSE_SCSI_READ_CAPACITY_10_PARAMETER_DATA
	------------------------------------------------
*/
class ATOSE_scsi_read_capacity_10_parameter_data
{
public:
	ATOSE_msb_uint32_t returned_logical_block_address;
	ATOSE_msb_uint32_t block_length_in_bytes;
} ;

#endif
