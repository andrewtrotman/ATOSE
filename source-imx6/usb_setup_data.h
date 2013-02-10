/*
	USB_SETUP_DATA.H
	----------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_SETUP_DATA_H_
#define USB_SETUP_DATA_H_

#include <stdint.h>
#include "usb_setup_data_request_type.h"

/*
	class ATOSE_USB_SETUP_DATA
	--------------------------
	Page 248 of "Universal Serial Bus Specification Revision 2.0"
	This structure is the shape of the setup packet sent from the host to the device.
	It is used in communications before the device comes on line
*/
class ATOSE_usb_setup_data
{
public:
	/*
		The various bits in the bmRequestType member of a setup packet
		See page 248 of "Universal Serial Bus Specification Revision 2.0"
	*/
	static const uint8_t DIRECTION_HOST_TO_DEVICE = 0x00;
	static const uint8_t DIRECTION_DEVICE_TO_HOST = 0x10;

	static const uint8_t TYPE_STANDARD  = 0x00;
	static const uint8_t TYPE_CLASS = 0x01;
	static const uint8_t TYPE_VENDOR = 0x02;
	static const uint8_t TYPE_RESERVED = 0x03;

	static const uint8_t RECIPIENT_DEVICE = 0x00;
	static const uint8_t RECIPIENT_INTERFACE = 0x01;
	static const uint8_t RECIPIENT_ENDPOINT = 0x02;
	static const uint8_t RECIPIENT_OTHER = 0x03;

public:
	ATOSE_usb_setup_data_request_type bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__ ((packed));

#endif
