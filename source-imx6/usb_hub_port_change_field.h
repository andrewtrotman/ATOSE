/*
	USB_HUB_PORT_CHANGE_FIELD.H
	---------------------------
*/

#ifndef USB_HUB_PORT_CHANGE_FIELD_H_
#define USB_HUB_PORT_CHANGE_FIELD_H_

/*
	class ATOSE_USB_HUB_PORT_CHANGE_FIELD
	-------------------------------------
*/
class ATOSE_usb_hub_port_change_field
{
public:
	union
		{
		uint16_t wPortChange;
		struct
			{
			unsigned c_port_connection   : 1;
			unsigned c_port_enable       : 1;
			unsigned c_port_suspect      : 1;
			unsigned c_port_over_current : 1;
			unsigned c_port_reset        : 1;
			unsigned reserved            : 11;
			};
		};
};

#endif
