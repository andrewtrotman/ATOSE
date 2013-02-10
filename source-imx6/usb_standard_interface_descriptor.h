/*
	USB_STANDARD_INTERFACE_DESCRIPTOR.H
	-----------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_INTERFACE_DESCRIPTOR_H_
#define USB_STANDARD_INTERFACE_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_STANDARD_INTERFACE_DESCRIPTOR
	---------------------------------------------
	Page 268-269 of "Universal Serial Bus Specification Revision 2.0"
	Each configuration has one or more interfaces.
*/
class ATOSE_usb_standard_interface_descriptor
{
public:
	/*
		The different interface classes we know about
	*/
	static const uint8_t CLASS_CDC = 0x02;
	static const uint8_t CLASS_DATA = 0x0A;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__ ((packed));

#endif
