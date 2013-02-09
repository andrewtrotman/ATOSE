/*
	USB_MS_COMPATIBLE_ID_FUNCTION.H
	-------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Microsoft extensions to the USB protocols
*/
#ifndef USB_MS_COMPATIBLE_ID_FUNCTION_H_
#define USB_MS_COMPATIBLE_ID_FUNCTION_H_

#include <stdint.h>

/*
	class ATOSE_USB_MS_COMPATIBLE_ID_FUNCTION
	-----------------------------------------
	Page 7 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
*/
class ATOSE_usb_ms_compatible_id_function
{
public:
	uint8_t bFirstInterfaceNumber;
	uint8_t reserved1;
	uint8_t compatibleID[8];
	uint8_t subCompatibleID[8];
	uint8_t reserved2[6];
} __attribute__ ((packed));

#endif
