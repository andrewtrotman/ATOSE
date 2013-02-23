/*
	USB_STANDARD_HUB_DESCRIPTOR.H
	-----------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_STANDARD_HUB_DESCRIPTOR_H_
#define USB_STANDARD_HUB_DESCRIPTOR_H_


/*
	class ATOSE_USB_STANDARD_HUB_DESCRIPTOR
	---------------------------------------
*/
class ATOSE_usb_standard_hub_descriptor
{
public:
	uint8_t bDescLength;
	uint8_t bDescriptorType;
	uint8_t bNbrPorts;
	uint16_t wHubCharacteristics;
	uint8_t bPwrOn2PwrGood;
	uint8_t bHubContrCurrent;
	uint8_t DeviceRemovable[32];
	//uint8_t PortPwrCtrlMask[32];	// depricated by USB version 1.0
} ;

#endif /* USB_STANDARD_HUB_DESCRIPTOR_H_ */
