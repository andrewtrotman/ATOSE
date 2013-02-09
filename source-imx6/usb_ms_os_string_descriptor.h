/*
	USB_MS_OS_STRING_DESCRIPTOR.H
	-----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_OS_STRING_DESCRIPTOR_H_
#define USB_MS_OS_STRING_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_MS_OS_STRING_DESCRIPTOR
	---------------------------------------
	Page 7 of "Microsoft OS Descriptors Overview (July 13, 2012)"
*/
class ATOSE_usb_ms_os_string_descriptor
{
public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t qwSignature[14];
	uint8_t bMS_VendorCode;
	uint8_t bPad;
} __attribute__ ((packed));

#endif
