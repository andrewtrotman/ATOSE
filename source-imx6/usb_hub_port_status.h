/*
	USB_HUB_PORT_STATUS.H
	---------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_HUB_PORT_STATUS_H_
#define USB_HUB_PORT_STATUS_H_

/*
	class ATOSE_USB_HUB_PORT_STATUS
	-------------------------------
*/
class ATOSE_usb_hub_port_status
{
public:
	union
		{
		uint16_t wPortStatus;
		struct
			{
			unsigned port_connection   : 1;
			unsigned port_enable       : 1;
			unsigned port_suspect      : 1;
			unsigned port_over_current : 1;
			unsigned port_reset        : 1;
			unsigned reserved1         : 3;
			unsigned port_power        : 1;
			unsigned port_low_speed    : 1;
			unsigned port_high_speed   : 1;
			unsigned port_test         : 1;
			unsigned port_indicator    : 1;
			unsigned reserved2         : 3;
			};
		};
};

#endif
