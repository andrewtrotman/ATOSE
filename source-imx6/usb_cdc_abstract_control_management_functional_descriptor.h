/*
	USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR.H
	-----------------------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_H_
#define USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	---------------------------------------------------------------------
	Page 35-36 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
class ATOSE_usb_cdc_abstract_control_management_functional_descriptor
{
public:
	static const uint8_t CAPABILITY_CONNECT = 0x08; // supports Network_Connection
	static const uint8_t CAPABILITY_BREAK = 0x04;   // supports Send_Break
	static const uint8_t CAPABILITY_LINE = 0x02;    // supports Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, Serial_State
	static const uint8_t CAPABILITY_FEATURE = 0x01; // supports Set_Comm_Feature, Clear_Comm_Feature, Get_Comm_Feature

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bmCapabilities;
} __attribute__ ((packed));

#endif
