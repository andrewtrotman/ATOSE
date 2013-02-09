/*
	USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_DTD_TOKEN.H
	--------------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_DTD_TOKEN_H_
#define USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_DTD_TOKEN_H_

#include <stdint.h>

/*
	union ATOSE_USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_DTD_TOKEN
	------------------------------------------------------------
*/
typedef union
{
uint32_t all;
struct
	{
	unsigned status      : 8;
	unsigned reserved0   : 2;
	unsigned mult0       : 2;
	unsigned reserved1   : 3;
	unsigned ioc         : 1;
	unsigned total_bytes : 15;
	unsigned reserved2   : 1;
	} bit __attribute__ ((packed));
} ATOSE_usb_imx6q_endpoint_transfer_descriptor_dtd_token;

#endif
