/*
	CLOCK_IMX6Q.H
	--------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Code to control the Freescale i.MX6Q Enhanced Periodic Interrupt Timer (EPIT)  That timer is discussed in
	Chapter 24 (pages 1165-1178 of "i.MX 6Dual/6Quad Applications Processor Reference Manual, Rev. 0, 11/2012"
	The primary concern here is to provide a u-second scale timer.
*/
#ifndef CLOCK_IMX6Q_H_
#define CLOCK_IMX6Q_H_

#include <stdint.h>

/*
	class ATOSE_CLOCK_IMX6Q
	-----------------------
*/
class ATOSE_clock_imx6q
{
private:
	static const uint32_t us_clock = 1;		// the i.MX6Q has two EPIT timers, we're going to use EPIT #1 as the u-second timer

public:
	ATOSE_clock_imx6q();
	static void delay_us(uint32_t time_in_useconds);
} ;

#endif

