/*
	USB_EHCI_QUEUE_HEAD.H
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_HEAD_H_
#define USB_EHCI_QUEUE_HEAD_H_

#include <stdint.h>

#include "usb_ehci_queue_head_endpoint_capabilities.h"
#include "usb_ehci_queue_element_transfer_descriptor.h"
#include "usb_ehci_queue_head_horizontal_link_pointer.h"
#include "usb_ehci_queue_head_endpoint_characteristics.h"
#include "usb_ehci_queue_element_transfer_descriptor_token.h"

/*
	class ATOSE_USB_EHCI_QUEUE_HEAD
	-------------------------------
	See page 5215-5216 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_queue_head
{
public:
	static const uint32_t BUFFER_POINTERS = 5;

public:
	ATOSE_usb_ehci_queue_head_horizontal_link_pointer queue_head_horizontal_link_pointer;
	ATOSE_usb_ehci_queue_head_endpoint_characteristics characteristics;
	ATOSE_usb_ehci_queue_head_endpoint_capabilities capabilities;
	ATOSE_usb_ehci_queue_element_transfer_descriptor *current_qtd_pointer;
	ATOSE_usb_ehci_queue_element_transfer_descriptor *next_qtd_pointer;
	ATOSE_usb_ehci_queue_element_transfer_descriptor *alternate_next_qtd_pointer;
	ATOSE_usb_ehci_queue_element_transfer_descriptor_token token;
	void *buffer_pointer[BUFFER_POINTERS];
} __attribute__ ((packed));

#endif
