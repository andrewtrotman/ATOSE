/*
	USB_STRING_DESCRIPTOR.H
	-----------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STRING_DESCRIPTOR_H_
#define USB_STRING_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_STRING_DESCRIPTOR
	---------------------------------
	Page 273 of "Universal Serial Bus Specification Revision 2.0"
	When we return a string to the host its in one of these objects
*/
class ATOSE_usb_string_descriptor
{
public:
	/*
		String constants. Only LANGUAGE is cast is stone, the others are user configurable
	*/
	static const uint8_t NONE = 0x00;
	static const uint8_t LANGUAGE = 0x00;		// See page 273 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
	static const uint8_t MANUFACTURER = 0x01;
	static const uint8_t PRODUCT = 0x02;
	static const uint8_t SERIAL_NUMBER = 0x03;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t wString[32];		// hopefully 32 is enough (recall that they are Unicode strings so 16 characters
}  __attribute__ ((packed));

#endif
