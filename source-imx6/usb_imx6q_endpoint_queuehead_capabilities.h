/*
	USB_IMX6Q_ENDPOINT_QUEUEHEAD_CAPABILITIES.H
	-------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_ENDPOINT_QUEUEHEAD_CAPABILITIES_H_
#define USB_IMX6Q_ENDPOINT_QUEUEHEAD_CAPABILITIES_H_

#include <stdint.h>

/*
	union ATOSE_USB_IMX6Q_ENDPOINT_QUEUEHEAD_CAPABILITIES
	-----------------------------------------------------
*/
typedef union
{
uint32_t all;
struct
	{
	unsigned reserved0             : 15;
	unsigned ios                   : 1;
	unsigned maximum_packet_length : 11;
	unsigned reserved1             : 2;
	unsigned zlt                   : 1;
	unsigned mult                  : 2;  // not sure if this exists on the i.MX233
	} bit __attribute__ ((packed));
} ATOSE_usb_imx6q_endpoint_queuehead_capabilities;


#endif
