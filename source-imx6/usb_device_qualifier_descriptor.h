/*
	USB_DEVICE_QUALIFIER_DESCRIPTOR.H
	---------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_DEVICE_QUALIFIER_DESCRIPTOR_H_
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_DEVICE_QUALIFIER_DESCRIPTOR
	-------------------------------------------
	Page 262-263 of "Universal Serial Bus Specification Revision 2.0"
	This gets used when a high-speed device is plugged in so that the host
	can determine what would happen if plugged into a non-high-speed port.
*/
class ATOSE_usb_device_qualifier_descriptor
{
public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint8_t bNumConfigurations;
	uint8_t reserved;
} __attribute__ ((packed));

#endif

