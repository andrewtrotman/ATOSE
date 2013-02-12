/*
	USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_TOKEN.H
	--------------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_TOKEN_H_
#define USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_TOKEN_H_

#include <stdint.h>

/*
	class ATOSE_USB_EHCI_QUEUE_ELEMENT_TRANSFER_DESCRIPTOR_TOKEN
	------------------------------------------------------------
	See page 5211 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_queue_element_transfer_descriptor_token
{
public:
	static const uint32_t PID_OUT = 0x00;
	static const uint32_t PID_IN = 0x01;
	static const uint32_t PID_SETUP = 0x02;

	static const uint32_t STATUS_ACTIVE 1<<7;
	static const uint32_t STATUS_HALTED 1<<6;
	static const uint32_t STATUS_DATA_BUFFER_ERROR 1<<5;
	static const uint32_t STATUS_BABBLE 1<<4;
	static const uint32_t STATUS_TRANSACTION_ERROR 1<<3;
	static const uint32_t STATUS_MISSED_MICROFRAME 1<<2;
	static const uint32_t STATUS_SPLIT_TRANSACTION_STATE 1<<1;
	static const uint32_t STATUS_PING_STATE 1<<0;

public:
	union
		{
		uint32_t all;
		struct
			{
			unsigned status      : 8;
			unsigned pid_code    : 2;
			unsigned c_err			: 2;
			unsigned c_page		: 3;
			unsigned ioc			: 1;
			unsigned total_bytes : 15;
			unsigned dt				: 1;
			} bit __attribute__ ((packed));
		} ;
} ;

#endif


