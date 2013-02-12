/*
	USB_EHCI_QUEUE_HEAD_HORIZONTAL_LINK_POINTER.H
	---------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_H_
#define USB_EHCI_QUEUE_HEAD_HORIZONTAL_LINK_POINTER_H_

/*
	class ATOSE_USB_EHCI_QUEUE_HEAD_HORIZONTAL_LINK_POINTER
	-------------------------------------------------------
	See page 5216-5217 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

*/
class ATOSE_usb_ehci_queue_head_horizontal_link_pointer
{
public:
	/*
		typ values
	*/
	static const uint32_t ISOCHRONOUS_TRANSFER_DESCRIPTOR 0x00
	static const uint32_t QUEUE_HEAD = 0x01
	static const uint32_t SPLIT_TRANSACTION_ISOCHRONOUS_TRANSFER_DESCRIPTOR = 0x02;
	static const uint32_t FRAME_SPAN_TRAVERSAL_NODE = 0x03;

	/*
		t values
	*/
	static const uint32_t CONTINUATION = 1;
	static const uint32_t TERMINATOR = 1;

public:
	union
		{
		uint32_t all;
		struct
			{
			unsigned t : 1;
			unsigned typ : 2;
			unsigned zero : 2;
			unsigned qhlp : 27;
			} bit __attribute__ ((packed));
		};
} ;

#endif
