/*
	USB_MS.H
	--------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_H_
#define USB_MS_H_

/*
	class ATOSE_USB_MS
	------------------
	The purpose of this class is to give a namespace to the constants it contains
*/
class ATOSE_usb_ms
{
public:
	/*
		The version of the Microsoft "standard" that we're compatible with
	*/	
	static const uint16_t VERSION = 0x0100;	// We're compatible with Microsoft Extensions version 1.0

	/*
		The different descriptor types we know about
	*/
	static const uint16_t STRING_OS_DESCRIPTOR = 0xEE;		// See page 6 of "Microsoft OS Descriptors Overview"
	static const uint16_t DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID = 0x04;
	static const uint16_t DESCRIPTOR_TYPE_EXTENDED_PROPERTIES= 0x05;

	/*
		Property types
	*/
	static const uint32_t REG_SZ = 0x01;		// zero terminated Unicode string

	/*
		and the different request types
	*/
	static const uint16_t REQUEST_GET_EXTENDED_COMPAT_ID_OS_FEATURE_DESCRIPTOR 0xC0
	static const uint16_t REQUEST_GET_EXTENDED_PROPERTIES_OS_DESCRIPTOR 0xC1
} ;

#endif
