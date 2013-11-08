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
private:
	static const uint8_t MAX_TRANSFER_SIZE = 8;	// the maximum number of bytes we've seen in a descriptor of a mouse

private:
	uint8_t endpoint;							// the interrupt endpoint
	uint8_t transfer_size;					// the number of bytes transferred in an interrupt transfer
	uint8_t buffer[MAX_TRANSFER_SIZE];	// the maximum number of bytes we've seen in a mouse interrupt transfer is 6
	uint8_t axies;								// number of axies the mouse has (x, y, z, h)
	int32_t pointer_x, pointer_y;			// current x and y location on the screen.
	uint8_t buttons;							// current button state

protected:
	virtual void acknowledge(void);
	
public:
	ATOSE_host_usb_device_hid_mouse(ATOSE_host_usb_device *details);
} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_hid_mouse), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_hid_mouse");

#endif
