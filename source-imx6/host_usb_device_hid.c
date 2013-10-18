/*
	HOST_USB_DEVICE_HID.C
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "host_usb_device_hid.h"

/*
	ATOSE_HOST_USB_DEVICE_HID::GET_HID_DESCRIPTOR()
	-----------------------------------------------
*/
uint32_t ATOSE_host_usb_device_hid::get_hid_descriptor(void *descriptor, uint32_t descriptor_length)
{
return send_setup_packet(0x81, ATOSE_usb::REQUEST_GET_DESCRIPTOR, ATOSE_usb::DESCRIPTOR_TYPE_REPORT << 8, 0, descriptor_length, descriptor);
}

/*
	ATOSE_HOST_USB_DEVICE_HID::ATOSE_HOST_USB_DEVICE_HID()
	------------------------------------------------------
*/
ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid(ATOSE_host_usb_device *details) : ATOSE_host_usb_device(details)
{
}
