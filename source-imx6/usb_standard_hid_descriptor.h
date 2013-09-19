/*
	USB_STANDARD_HID_DESCRIPTOR.H
	-----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_HID_DESCRIPTOR_H_
#define USB_STANDARD_HID_DESCRIPTOR_H_


/*
	class ATOSE_USB_STANDARD_HID_DESCRIPTOR
	---------------------------------------
*/
class ATOSE_usb_standard_hid_descriptor
{
public:
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bClassDescriptorType;
	uint16_t wItemLength;
} ;

#endif
