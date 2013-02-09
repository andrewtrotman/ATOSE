/*
	USB_IMX6Q_ENDPOINT_QUEUEHEAD.H
	------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_ENDPOINT_QUEUEHEAD_H_
#define USB_IMX6Q_ENDPOINT_QUEUEHEAD_H_

#include <stdint.h>

/*
	class ATOSE_USB_IMX6Q_ENDPOINT_QUEUEHEAD
	----------------------------------------
	Page 5330 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	This structure must be 64-byte aligned.  To make sure an array of
	these is correctly alligned its necessary to pad the end of the
	structure with a few extra words
*/
class ATOSE_usb_imx6q_endpoint_queuehead
{
public:
	ATOSE_usb_imx6q_endpoint_queuehead_capabilities capabilities;		/* capability information about the endpoint */
	ATOSE_usb_imx6q_endpoint_transfer_descriptor *current_dtd_pointer;	/* Current dTD Pointer(31-5). This is set by the USB controller */
	ATOSE_usb_imx6q_endpoint_transfer_descriptor dtd_overlay_area;		/* The hardware copies the current transfer descriptor here when it actions it. */
	ATOSE_usb_setup_data setup_buffer;									/* Setup packets are copies here rather than into the transfer descriptor buffers */
	uint32_t reserved2[4];												/* Needed to guarantee 64-byte allignment */
} __attribute__ ((packed));

#endif
