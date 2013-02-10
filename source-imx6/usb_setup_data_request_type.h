/*
	USB_SETUP_DATA_REQUEST_TYPE.H
	-----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_SETUP_DATA_REQUEST_TYPE_H_
#define USB_SETUP_DATA_REQUEST_TYPE_H_

#include <stdint.h>

/*
	class ATOSE_USB_SETUP_DATA_REQUEST_TYPE
	---------------------------------------
	Page 248 of "Universal Serial Bus Specification Revision 2.0"
	This union is used to decode the meaning of the bmRequestType memeber of
	a setup packet (see declaration of usb_setup_data)
*/
class ATOSE_usb_setup_data_request_type
{
public:
	union
	{
	uint8_t all;
	struct
		{
		unsigned recipient : 5;
		unsigned type      : 2;
		unsigned direction : 1;
		} __attribute__ ((packed)) bit;
	};
} ;

#endif

