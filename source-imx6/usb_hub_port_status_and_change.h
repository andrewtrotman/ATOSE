/*
	USB_HUB_PORT_STATUS_AND_CHANGE.H
	--------------------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_HUB_PORT_STATUS_AND_CHANGE_H_
#define USB_HUB_PORT_STATUS_AND_CHANGE_H_

#include "usb_hub_port_status.h"
#include "usb_hub_port_change_field.h"

/*
	class ATOSE_USB_HUB_PORT_STATUS_AND_CHANGE
	------------------------------------------
*/
class ATOSE_usb_hub_port_status_and_change
{
public:
	ATOSE_usb_hub_port_status status;
	ATOSE_usb_hub_port_change_field change;
};

#endif
