/*
	USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR.H
	-----------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_H_
#define USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	---------------------------------------------------------
	Page 34-35 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
class ATOSE_usb_cdc_call_management_functional_descriptor
{
public:
	static const uint8_t CAPABILITY_NONE = 0x00;
	static const uint8_t CAPABILITY_CM_COMUNICATIONS = 0x01;
	static const uint8_t CAPABILITY_CM_DATA = 0x03;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bmCapabilities;
	uint8_t bDataInterface;
} __attribute__ ((packed));

#endif
