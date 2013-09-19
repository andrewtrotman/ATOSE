/*
	HOST_USB_DEVICE_HID.C
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "host_usb_device_hid.h"

#include "debug_kernel.h"

/*
	ATOSE_HOST_USB_DEVICE_HID::ATOSE_HOST_USB_DEVICE_HID()
	------------------------------------------------------
*/
ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid(ATOSE_host_usb_device *details) : ATOSE_host_usb_device(details)
{
static unsigned char desc[1024];
static char buffer[4];
dead = false;

buffer[0] = 0xFF;
buffer[1] = 0xFE;
buffer[2] = 0xFD;
buffer[3] = 0xFC;


memset(desc, 0, sizeof(desc));
get_hid_descriptor(desc, sizeof(desc));
debug_dump_buffer(desc, 0, 10);


max_packet_size[1] = 8;
debug_print_string("enter ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid\r\n");
while (1)
	{
	details->ehci->read_interrupt_packet(this, 1, buffer, sizeof(buffer));
	debug_dump_buffer((unsigned char *)buffer, 0, sizeof(buffer));
	debug_print_string("\r\n");
	}
debug_print_string("exit ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid\r\n");
}
