/*
	USB_EHCI_QUEUE_HEAD_ENDPOINT_CAPABILITIES.H
	-------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_HEAD_ENDPOINT_CAPABILITIES_H_
#define USB_EHCI_QUEUE_HEAD_ENDPOINT_CAPABILITIES_H_

/*
	class ATOSE_USB_EHCI_QUEUE_HEAD_ENDPOINT_CAPABILITIES
	-----------------------------------------------------
	See page 5218-5219 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_queue_head_endpoint_capabilities
{
public:
	static const uint32_t TRANSACTIONS_ONE  = 0x01; // One transaction to be issued for this endpoint per micro-frame
	static const uint32_t TRANSACTIONS_TWO  = 0x02; // Two transactions to be issued for this endpoint per micro-frame
	static const uint32_t TRANSACTIONS_TREE = 0x03;	// Three transactions to be issued for this endpoint per micro-frame

public:
	union
		{
		uint32_t all;
		struct
			{
			unsigned u_frame_s_mask : 8;
			unsigned u_frame_c_mask : 8;
			unsigned hib_addr       : 7;
			unsigned port_number    : 7;
			unsigned mult           : 2;
			} bit __attribute__ ((packed));
		} ;
} __attribute__ ((packed));

#endif

