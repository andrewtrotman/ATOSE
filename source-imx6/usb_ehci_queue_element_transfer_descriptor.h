/*
	USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR.H
	--------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_H_
#define USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_H_

#include <stdint.h>

#include "usb_ehci_queue_element_transfer_descriptor_token.h"

/*
	class ATOSE_USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR
	------------------------------------------------------
	See page 5210 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_queue_element_transfer_descriptor
{
public:
	static const uint32_t BUFFER_POINTERS = 5;
	static const uint32_t BUFFER_SIZE = 4096;
	static const uint32_t TERMINATOR = 1;

public:
	ATOSE_usb_ehci_queue_element_transfer_descriptor *next_qtd_pointer;
	ATOSE_usb_ehci_queue_element_transfer_descriptor *alternate_next_qtd_pointer;
	ATOSE_usb_ehci_queue_element_transfer_descriptor_token token;
	void *buffer_pointer[BUFFER_POINTERS];
} __attribute__ ((packed));

#endif
