/*
	USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR.H
	--------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR_H_
#define USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR
	------------------------------------------------
	Page 34 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
class ATOSE_usb_cdc_header_functional_descriptor
{
public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint16_t bcdCDC;
} __attribute__ ((packed));

#endif
