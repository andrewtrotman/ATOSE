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
public:
	uint8_t endpoint_in;
	uint8_t endpoint_out;
	uint8_t logical_units;		// the number of LUNs.  We'll only support those with 1 LUN.

public:
	ATOSE_host_usb_device_disk(ATOSE_host_usb_device *details);

	/*
		These methods all retun 0 on success
	*/
	uint32_t clear_feature_halt_disk(uint32_t endpoint) { return send_setup_packet(0x02, ATOSE_usb::REQUEST_CLEAR_FEATURE, endpoint); }
	uint32_t reset_disk(void)                           { return send_setup_packet(0x21, 0xFF); }
	uint32_t get_max_lun(uint8_t *luns)                 { return send_setup_packet(0xA1, 0xFE, 0, 0, 1, &luns); }

	/*
		Experimental disk stuff
	*/
	uint32_t get_disk_inquiry(void);
} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_disk), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_disk");

#endif
