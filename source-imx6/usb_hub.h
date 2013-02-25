/*
	USB_HUB.H
	---------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_HUB_H_
#define USB_HUB_H_

/*
	class ATOSE_USB_HUB
	-------------------
*/
class ATOSE_usb_hub
{
public:
	static const uint32_t C_HUB_LOCAL_POWER = 0;
	static const uint32_t C_HUB_OVER_CURRENT = 1;
	static const uint32_t PORT_CONNECTION = 0;
	static const uint32_t PORT_ENABLE = 1;
	static const uint32_t PORT_SUSPEND = 2;
	static const uint32_t PORT_OVER_CURRENT = 3;
	static const uint32_t PORT_RESET = 4;
	static const uint32_t PORT_POWER = 8;
	static const uint32_t PORT_LOW_SPEED = 9;
	static const uint32_t C_PORT_CONNECTION = 16;
	static const uint32_t C_PORT_ENABLE = 17;
	static const uint32_t C_PORT_SUSPEND = 18;
	static const uint32_t C_PORT_OVER_CURRENT = 19;
	static const uint32_t C_PORT_RESET = 20;
	static const uint32_t PORT_TEST = 21;
	static const uint32_t PORT_INDICATOR = 22;

	static const uint32_t REQUEST_RESET_TT = 0x09;
} ;

#endif /* USB_HUB_H_ */
