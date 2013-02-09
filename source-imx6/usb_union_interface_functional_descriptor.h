/*
	USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR.H
	-------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR_H_
#define USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR
	-----------------------------------------------------
	Page 40-41 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
class ATOSE_usb_union_interface_functional_descriptor
{
public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bMasterInterface;
	uint8_t bSlaveInterface0;
} __attribute__ ((packed));

#endif
