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
	static const uint8_t CLASS_HUB = 0x09;

	/*
		USB Hub protocols.  A "Transaction Translator" or "TT" is a device that slows transmission speeds down from
		USB 2.0 (and presumably above) to the slower speeds needed for USB 1 and USB 1.1.  The more the merrier.
	*/
	static const uint8_t PROTOCOL_HUB_TRANSACTION_TRANSLATOR_NONE = 0x00;
	static const uint8_t PROTOCOL_HUB_TRANSACTION_TRANSLATOR_ONE = 0x01;
	static const uint8_t PROTOCOL_HUB_TRANSACTION_TRANSLATOR_MANY = 0x02;

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
