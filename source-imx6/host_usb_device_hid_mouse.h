/*
	HOST_USB_DEVICE_HID_MOUSE.H
	---------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_DEVICE_HID_MOUSE_H_
#define HOST_USB_DEVICE_HID_MOUSE_H_

#include "host_usb_device_hid.h"

/*
	class ATOSE_HOST_USB_DEVICE_HID_MOUSE
	-------------------------------------
*/
class ATOSE_host_usb_device_hid_mouse : public ATOSE_host_usb_device_hid
{
public:
	ATOSE_host_usb_device_hid_mouse(ATOSE_host_usb_device *details);
} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_hid_mouse), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_hid_mouse");

#endif
