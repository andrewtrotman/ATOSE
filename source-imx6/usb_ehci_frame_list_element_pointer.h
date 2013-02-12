/*
	USB_EHCI_FRAME_LIST_ELEMENT_POINTER.H
	-------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_FRAME_LIST_ELEMENT_POINTER_H_
#define USB_EHCI_FRAME_LIST_ELEMENT_POINTER_H_

#include <stdint.h>

/*
	class ATOSE_USB_EHCI_FRAME_LIST_ELEMENT_POINTER
	-----------------------------------------------
	See page 5199 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_frame_list_element_pointer
{
public:
	static const isochronous_transfer_descriptor = 0;
	static const queue_head = 1;
	static const split_transaction_isochronous_transfer_descriptor = 2;
	static const frame_span_traversal_node = 3;

public:
	union
		{
		uint32_t all;
		struct
			{
			unsigned reserved : 1;
			unsigned typ : 2;
			unsigned zero : 2;
			unsigned frame_list_link_pointer : 27;
			} bit __attribute__ ((packed));
		} ;
} ;

#endif



