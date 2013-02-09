/*
	USB_STANDARD_DEVICE_DESCRIPTOR.H
	--------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_DEVICE_DESCRIPTOR_H_
#define USB_STANDARD_DEVICE_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_STANDARD_DEVICE_DESCRIPTOR
	------------------------------------------
	Page 262-263 of "Universal Serial Bus Specification Revision 2.0"
	This structure is used by the device to tell the host what the device is
*/
class ATOSE_usb_standard_device_descriptor
{
public:
	/*
		The different device subclasses we know about
	*/
	static const uint8_t SUBCLASS_NONE = 0x00;

	/*
		The different device protocols we know about
	*/
	static const uint8_t PROTOCOL_NONE = 0x00;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__ ((packed));

#endif
