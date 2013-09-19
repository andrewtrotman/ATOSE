/*
	HOST_USB_DEVICE_HID.H
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_DEVICE_HID_H_
#define HOST_USB_DEVICE_HID_H_

#include "host_usb_device.h"
#include "host_usb_device_generic.h"

/*
	class ATOSE_HOST_USB_DEVICE_HID
	-------------------------------
*/
class ATOSE_host_usb_device_hid : public ATOSE_host_usb_device
{
public:
	ATOSE_host_usb_device_hid(ATOSE_host_usb_device *details);
	uint32_t get_hid_descriptor(unsigned char *descriptor, uint32_t size) { return get_descriptor(0x81, ATOSE_usb::DESCRIPTOR_TYPE_REPORT, descriptor, size); }

} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_hid), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_hid");

#endif
