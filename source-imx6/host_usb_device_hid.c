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
if (device_protocol == PROTOCOL_MOUSE)
	debug_print_string("\r\n\r\nHID::MOUSE\r\n\r\n");
if (device_protocol == PROTOCOL_KEYBOARD)
	debug_print_string("\r\n\r\nHID::KEYBOARD\r\n\r\n");

max_packet_size[1] = 8;

static unsigned char desc[0x700];
static char buffer[8];
dead = false;

memset(desc, 0, sizeof(desc));
get_hid_descriptor(desc, sizeof(desc));
debug_dump_buffer(desc, 0, sizeof(desc));

debug_print_string("enter ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid\r\n");
while (1)
	{
	details->ehci->read_interrupt_packet(this, 1, buffer, sizeof(buffer));
	debug_dump_buffer((unsigned char *)buffer, 0, sizeof(buffer));
	debug_print_string("\r\n");
	}
debug_print_string("exit ATOSE_host_usb_device_hid::ATOSE_host_usb_device_hid\r\n");
}
