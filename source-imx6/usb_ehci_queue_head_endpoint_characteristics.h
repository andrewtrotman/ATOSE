/*
	USB_EHCI_QUEUE_HEAD_ENDPOINT_CHARACTERISTICS.H
	----------------------------------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/

#ifndef USB_EHCI_QUEUE_HEAD_ENDPOINT_CHARACTERISTICS.H
#define USB_EHCI_QUEUE_HEAD_ENDPOINT_CHARACTERISTICS.H

/*
	class ATOSE_USB_EHCI_QUEUE_HEAD_ENDPOINT_CHARACTERISTICS
	--------------------------------------------------------
	See page 5217-5218 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
class ATOSE_usb_ehci_queue_head_endpoint_characteristics
{
public:
	static const uint32_t MAX_PACKET_SIZE = 1024;
	static const uint32_t SPEED_FULL = 0x00;	// ( 12  Mbits/sec)
	static const uint32_t SPEED_LOW  = 0x01;	// (  1.5 Mbits/sec)
	static const uint32_t SPEED_HIGH = 0x02;	// (480  Mbits/sec)

public:
	union
		{
		uint32_t all;
		struct
			{
			unsigned device_address        : 7;
			unsigned i                     : 1;
			unsigned endpt                 : 4;
			unsigned ep                    : 2;
			unsigned dtc                   : 1;
			unsigned h                     : 1;
			unsigned maximum_packet_length : 11;
			unsigned c                     : 1;
			unsigned rl                    : 4;
			} bit __attribute__ ((packed));
		} ;
} ;

#endif
