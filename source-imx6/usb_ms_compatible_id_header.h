/*
	USB_MS_COMPATIBLE_ID_HEADER.H
	-----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_COMPATIBLE_ID_HEADER_H_
#define USB_MS_COMPATIBLE_ID_HEADER_H_

#include <stdint.h>

/*
	class ATOSE_USB_MS_COMPATIBLE_ID_HEADER
	---------------------------------------
	Page 6 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
*/
class ATOSE_usb_ms_compatible_id_header
{
public:
	uint32_t dwLength;
	uint16_t bcdVersion;
	uint16_t wIndex;
	uint8_t bCount;
	uint8_t reserved[7];
} __attribute__ ((packed));

#endif
