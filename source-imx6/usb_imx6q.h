/*
	USB_IMX6Q.H
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_H_
#define USB_IMX6Q_H_

#include <stdint.h>

/*
	We need to tell the i.MX6 SDK that we're using an i.MX6Q
*/
#define CHIP_MX6DQ 1

/*
	class ATOSE_USB_IMX6Q
	---------------------
*/
class ATOSE_usb_imx6q
{
public:
	/*
		a transfer can do at most 4096 bytes of memory per buffer (due to the way counting is done).
		See page 7 of "Simplified Device Data Structures for the High-End ColdFire Family USB Modules"
	*/
	static const uint32_t QUEUE_BUFFER_SIZE = 4096;

	/*
		There are i.MX233 queue heads
	*/
	/*
		The i.MX53 and i.MX6Q have 8 endpoints 0..7
			see page 5449 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
			see page 4956 of "i.MX53 Multimedia Applications Processor Reference Manual Rev. 2.1, 06/2012"
	*/
	static const uint32_t MAX_ENDPOINTS = 8;
}

#endif
