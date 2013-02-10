/*
	USB_CDC_VIRTUAL_COM_PORT.H
	--------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_VIRTUAL_COM_PORT_H_
#define USB_CDC_VIRTUAL_COM_PORT_H_

#include <stdint.h>
#include "usb_cdc_header_functional_descriptor.h"
#include "usb_cdc_abstract_control_management_functional_descriptor.h"
#include "usb_cdc_call_management_functional_descriptor.h"
#include "usb_standard_configuration_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_union_interface_functional_descriptor.h"

/*
	class ATOSE_USB_CDC_VIRTUAL_COM_PORT
	------------------------------------
	A virtual COM port has one configuration, two interfaces (data and control) and three
	endpoints (data in and data out count as two).
*/

class ATOSE_usb_cdc_virtual_com_port
{
public:
	/*
		Preferred endpoint use for a virtual COM port
	*/
	static const uint8_t ENDPOINT_CONTROL = 0; 							// Endpoint 0 is always control
	static const uint8_t ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT = 1;		// Endpoint 1 is control management to the host
	static const uint8_t ENDPOINT_SERIAL = 2;							// Endpoint 2 is for data

public:
	ATOSE_usb_standard_configuration_descriptor cd;
		ATOSE_usb_standard_interface_descriptor id0;
			ATOSE_usb_cdc_header_functional_descriptor fd1;
			ATOSE_usb_cdc_call_management_functional_descriptor fd2;
			ATOSE_usb_cdc_abstract_control_management_functional_descriptor fd3;
			ATOSE_usb_union_interface_functional_descriptor fd4;
				ATOSE_usb_standard_endpoint_descriptor ep2;
		ATOSE_usb_standard_interface_descriptor id1;
			ATOSE_usb_standard_endpoint_descriptor ep3;
			ATOSE_usb_standard_endpoint_descriptor ep4;
} __attribute__ ((packed));

#endif
