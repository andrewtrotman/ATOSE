/*
	USB_MS_EXTENDED_PROPERTY_FUNCTION.H
	-----------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_EXTENDED_PROPERTY_FUNCTION_H_
#define USB_MS_EXTENDED_PROPERTY_FUNCTION_H_

#include <stdint.h>

/*
	class ATOSE_USB_MS_CUSTOM_PROPERTY_FUNCTION
	-------------------------------------------
	Page 7 of "Extended Properties OS Feature Descriptor Specification (July 13, 2012)"
*/
class ATOSE_usb_ms_custom_property_function
{
public:
	uint32_t dwSize;
	uint32_t dwPropertyDataType;
	uint16_t wPropertyNameLength;
	uint8_t bPropertyName[12];				// "Label" in Unicode
	uint32_t dwPropertyDataLength;
	uint8_t bPropertyData[16];
} __attribute__ ((packed));

#endif
