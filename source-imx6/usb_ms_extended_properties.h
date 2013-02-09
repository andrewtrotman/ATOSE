/*
	USB_MS_EXTENDED_PROPERTIES.H
	----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_EXTENDED_PROPERTIES_H_
#define USB_MS_EXTENDED_PROPERTIES_H_

#include <stdint.h>

#include "usb_ms_extended_property_header.h"
#include "usb_ms_extended_property_function.h"

/*
	class ATOSE_USB_MS_EXTENDED_PROPERTIES
	--------------------------------------
	Page 6 of "Extended Properties OS Feature Descriptor Specification (July 13, 2012)"
	This can have several properties, but we only need one.
*/
class ATOSE_usb_ms_extended_properties
{
public:
	ATOSE_usb_ms_extended_property_header header;
	ATOSE_usb_ms_extended_property_function property1;
}  __attribute__ ((packed));

#endif
