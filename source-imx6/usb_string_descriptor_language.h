/*
	USB_STRING_DESCRIPTOR_LANGUAGE.H
	--------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STRING_DESCRIPTOR_LANGUAGE_H_
#define USB_STRING_DESCRIPTOR_LANGUAGE_H_

#include <stdint.h>

/*
	class ATOSE_USB_STRING_DESCRIPTOR_LANGUAGE
	------------------------------------------
	Page 273 of "Universal Serial Bus Specification Revision 2.0"
	The host will ask us what language our text strings are in, the device
	responds with one of these.  As some devices support multiple languages
	the response can include several languages.  We'll only worry about one
	language at the moment.
*/
class ATOSE_usb_string_descriptor_language
{
public:
	/*
		Language codes for English
	*/
	static const uint16_t ENGLISH_UNITED_STATES = 0x0409;
	static const uint16_t ENGLISH_UNITED_KINGDOM = 0x0809;
	static const uint16_t ENGLISH_AUSTRALIAN = 0x0c09;
	static const uint16_t ENGLISH_CANADIAN = 0x1009;
	static const uint16_t ENGLISH_NEW_ZEALAND = 0x1409;
	static const uint16_t ENGLISH_IRELAND = 0x1809;
	static const uint16_t ENGLISH_SOUTH_AFRICA = 0x1c09;
	static const uint16_t ENGLISH_JAMAICA = 0x2009;
	static const uint16_t ENGLISH_CARIBBEAN = 0x2409;
	static const uint16_t ENGLISH_BELIZE = 0x2809;
	static const uint16_t ENGLISH_TRINIDAD = 0x2c09;
	static const uint16_t ENGLISH_ZIMBABWE = 0x3009;
	static const uint16_t ENGLISH_PHILIPPINES = 0x3409;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wLANGID;
}  __attribute__ ((packed));

#endif
