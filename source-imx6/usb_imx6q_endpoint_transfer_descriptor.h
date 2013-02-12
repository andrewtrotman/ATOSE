/*
	USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR.H
	----------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_H_
#define USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR_H_

#include <stdint.h>

#include "usb_imx6q_endpoint_transfer_descriptor_dtd_token.h"

/*
	class ATOSE_USB_IMX6Q_ENDPOINT_TRANSFER_DESCRIPTOR
	--------------------------------------------------
	Page 5332 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	The Endpoint Transfer Descriptor data struct must be 32-bye aligned (and is 32-bytes long) hence the padding at the end
*/
class ATOSE_usb_imx6q_endpoint_transfer_descriptor
{
public:
	static const uint32_t BUFFER_POINTERS = 5;
	static const uint32_t TERMINATOR = 0x01;
	static const uint8_t STATUS_ACTIVE = 0x80;

public:
	ATOSE_usb_imx6q_endpoint_transfer_descriptor *next_link_pointer;		/* Next TD pointer(31-5), T(0) set indicate invalid */
	ATOSE_usb_imx6q_endpoint_transfer_descriptor_dtd_token token;
	void *buffer_pointer[BUFFER_POINTERS];
	uint32_t reserved;
} __attribute__ ((packed));

#endif
