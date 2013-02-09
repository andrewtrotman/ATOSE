/*
	USB_MS_EXTENDED_COMPATIBLE_ID_OS_FEATURE_DESCRIPTOR.H
	-----------------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_EXTENDED_COMPATIBLE_ID_OS_FEATURE_DESCRIPTOR_H_
#define USB_MS_EXTENDED_COMPATIBLE_ID_OS_FEATURE_DESCRIPTOR_H_

#include <stdint.h>

#include "usb_ms_compatible_id_header,h"
#include "usb_ms_compatible_id_function.h

/*
	class ATOSE_USB_MS__EXTENDED_COMPATIBLE_ID_OS_FEATURE_DESCRIPTOR
	----------------------------------------------------------------
	Page 6 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
	This structure can hold manty different ms_usb_function_section objects, but for
	what we need one is enough.
*/
class ATOSE_usb_ms_extended_compatible_id_os_feature_descriptor
{
public:
	usb_ms_compatible_id_header header;
	usb_ms_compatible_id_function section1;
} __attribute__ ((packed));

#endif
