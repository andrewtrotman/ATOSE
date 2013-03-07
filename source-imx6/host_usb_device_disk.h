/*
	HOST_USB_DEVICE_DISK.H
	----------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_DEVICE_DISK_H_
#define HOST_USB_DEVICE_DISK_H_

#include <stdint.h>

#include "host_usb_device_generic.h"

/*
	class ATOSE_HOST_USB_DEVICE_DISK
	--------------------------------
*/
class ATOSE_host_usb_device_disk : public ATOSE_host_usb_device
{
private:
	static uint8_t ATOSE_usb_scsi_test_unit_ready[];
	static uint8_t ATOSE_usb_scsi_read_capacity_10[];
	static uint8_t ATOSE_usb_scsi_read_capacity_16[];
	static uint8_t ATOSE_usb_scsi_read_10[];
	static uint8_t ATOSE_usb_scsi_read_16[];

public:
	uint64_t block_count;		// the number of blocks on the disk (0..block_count-1) is the valid range
	uint64_t block_size;			// the size of an individual disk block (in bytes)
	uint8_t logical_units;		// the number of LUNs.  We'll only support those with 1 LUN

	/*
		Necessary for talking USB to the disk
	*/
	uint8_t endpoint_in;
	uint8_t endpoint_out;

protected:
	uint32_t perform_transaction(uint8_t *command, void *buffer = NULL, uint32_t buffer_length = 0);
	uint32_t scsi_test_unit_ready(void) { return perform_transaction(ATOSE_usb_scsi_test_unit_ready); }
	uint32_t scsi_read_capacity_10(uint64_t *count, uint64_t *size);
	uint32_t scsi_read_capacity_16(uint64_t *count, uint64_t *size);
	uint32_t scsi_read(uint8_t *buffer, uint32_t buffer_length, uint64_t block, uint32_t blocks_to_read = 1);

public:
	ATOSE_host_usb_device_disk(ATOSE_host_usb_device *details);

	/*
		These methods all return 0 on success
	*/
	uint32_t clear_feature_halt_disk(uint32_t endpoint) { return send_setup_packet(0x02, ATOSE_usb::REQUEST_CLEAR_FEATURE, endpoint); }
	uint32_t reset_disk(void)                           { return send_setup_packet(0x21, 0xFF); }
	uint32_t get_max_lun(uint8_t *luns)                 { return send_setup_packet(0xA1, 0xFE, 0, 0, 1, &luns); }

	uint32_t read_sector(void *buffer, uint64_t sector, uint64_t number_of_sectors = 1) { return scsi_read((uint8_t *)buffer, 512, sector, number_of_sectors); }

	/*
		Experimental disk stuff
	*/
	uint32_t get_disk_inquiry(void);
} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_disk), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_disk");

#endif
