/*
	USB_STANDARD_ENDPOINT_DESCRIPTOR.H
	----------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_ENDPOINT_DESCRIPTOR_H_
#define USB_STANDARD_ENDPOINT_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_STANDARD_ENDPOINT_DESCRIPTOR
	--------------------------------------------
	Page 269-271 of "Universal Serial Bus Specification Revision 2.0"
	Each interface is attached to some number of endpoints.  An endpoint is
	just like a socket - it is where we speak in order to pass a message around.
*/
class ATOSE_usb_standard_endpoint_descriptor
{
public:
	/*
		Endpoint types
	*/
	static const uint8_t TYPE_CONTROL = 0x00;
	static const uint8_t TYPE_ISOCHRONOUS = 0x01;
	static const uint8_t TYPE_BULK = 0x02;
	static const uint8_t TYPE_INTERRUPT = 0x03;

	/*
		Endpoint directions (relative to the host (i.e. computer))
	*/
	static const uint8_t DIRECTION_IN = 0x80;
	static const uint8_t DIRECTION_OUT = 0x00;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;			// the endpont number
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__ ((packed));

#endif
