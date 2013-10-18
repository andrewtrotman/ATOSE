/*
	HOST_USB_DEVICE_HID_KEYBOARD.C
	------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "ascii_str.h"
#include "host_usb.h"
#include "host_usb_device_hid_keyboard.h"

#include "debug_kernel.h"


/*
	ATOSE_HOST_USB_DEVICE_HID_KEYBOARD::ATOSE_HOST_USB_DEVICE_HID_KEYBOARD
	----------------------------------------------------------------------
*/
ATOSE_host_usb_device_hid_keyboard::ATOSE_host_usb_device_hid_keyboard(ATOSE_host_usb_device *details) : ATOSE_host_usb_device_hid(details)
{
/*
max_packet_size[1] = 8;

static unsigned char desc[0x40];
static char buffer[8];
dead = false;

memset(desc, 0, sizeof(desc));
get_hid_descriptor(desc, sizeof(desc));
debug_dump_buffer(desc, 0, sizeof(desc));

while (1)
	{
	details->ehci->read_interrupt_packet(this, 1, buffer, sizeof(buffer));
	debug_dump_buffer((unsigned char *)buffer, 0, sizeof(buffer));
	debug_print_string("\r\n");
	}
*/
}