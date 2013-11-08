/*
	HOST_USB_DEVICE_HID_MOUSE.C
	---------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "ascii_str.h"
#include "host_usb.h"
#include "host_usb_device_hid_mouse.h"

#include "debug_kernel.h"

/*
	The format of a standard USB HID MOUSE packet is 4 bytes:

	+--------+--------+--------+--------+
	| button |   Y    |    X   |reportID|
	+--------+--------+--------+--------+

	Buttons is a bitstring of button number (set if down)

	High          Low
	+-+-+-+-+-+-+-+-+
	|8|7|6|5|4|3|2|1|
	+-+-+-+-+-+-+-+-+

	X and Y are in the range -127 to 127 and are a "displacement" value (i.e. relative change).

	In the case of a wheel mouse and extra "Z" (vertical displacement) is used giving 5 bytes.
	
	+--------+--------+--------+--------+--------+
	| button |   Y    |    X   |    Z   |reportID|
	+--------+--------+--------+--------+--------+
	
	In the case of a trackball mouse (e.g. the "Apple Optical USB Mouse"), Z is the vertical scroll wheel
	and H is the horizontal scroll wheel giving 6 bytes.  Both Z and H are displacements.

	+--------+--------+--------+--------+--------+--------+
	| button |   Y    |    X   |    Z   |    H   |reportID|
	+--------+--------+--------+--------+--------+--------+
*/

/*
	ATOSE_HOST_USB_DEVICE_HID_MOUSE::ATOSE_HOST_USB_DEVICE_HID_MOUSE
	----------------------------------------------------------------
*/
ATOSE_host_usb_device_hid_mouse::ATOSE_host_usb_device_hid_mouse(ATOSE_host_usb_device *details) : ATOSE_host_usb_device_hid(details)
{
/*
	Get the maximum packet length from the USB descriptor
*/
max_packet_size[1] = 8;
dead = false;
//details->ehci->queue_interrupt_transfer(this, 1, buffer, sizeof(buffer));

while (1)
	{
	details->ehci->read_interrupt_packet(this, 1, buffer, sizeof(buffer));
	debug_dump_buffer((unsigned char *)buffer, 0, sizeof(buffer));
	debug_print_string("\r\n");
	}

}

/*
	ATOSE_HOST_USB_DEVICE_HID_MOUSE::ACKNOWLEDGE()
	----------------------------------------------
*/
void ATOSE_host_usb_device_hid_mouse::acknowledge(void)
{
buttons = buffer[0];
pointer_y += (int8_t)buffer[1];
pointer_x += (int8_t)buffer[2];

//details->ehci->queue_interrupt_transfer(this, 1, buffer, sizeof(buffer));
}
