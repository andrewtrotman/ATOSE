/*
	HOST_USB_DEVICE_GENERIC.H
	-------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_DEVICE_GENERIC_H_
#define HOST_USB_DEVICE_GENERIC_H_

#include <stdint.h>
#include "host_usb_device.h"

/*
	class ATOSE_HOST_USB_DEVICE_GENERIC
	-----------------------------------
	NOTE: The padding here is used to create space so that an array of these objects can be
	allocated at startup as a memory placeholder.  Placement syntax is then used to actually
	create different descendant objects of ATOSE_host_usb_device
*/
class ATOSE_host_usb_device_generic : public ATOSE_host_usb_device
{
public:
	uint32_t padding[16];

public:
	ATOSE_host_usb_device_generic() : ATOSE_host_usb_device() {}
} ;

#endif

