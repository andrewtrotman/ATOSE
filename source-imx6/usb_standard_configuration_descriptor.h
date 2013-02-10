/*
	USB_STANDARD_CONFIGURATION_DESCRIPTOR.H
	---------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_CONFIGURATION_DESCRIPTOR_H_
#define USB_STANDARD_CONFIGURATION_DESCRIPTOR_H_

#include <stdint.h>

/*
	class ATOSE_USB_STANDARD_CONFIGURATION_DESCRIPTOR
	-------------------------------------------------
	Page 265 of "Universal Serial Bus Specification Revision 2.0"
	A device may have several configurations, the host selects which one to use
	by calling SetConfiguration() using the bConfigurationValue value

*/
class ATOSE_usb_standard_configuration_descriptor
{
public:
	static const uint8_t SELFPOWERED = 0xC0;
	static const uint8_t REMOTEWAKE = 0xA0;
	static const uint8_t mA_100 = 50;

public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__ ((packed));


#endif
