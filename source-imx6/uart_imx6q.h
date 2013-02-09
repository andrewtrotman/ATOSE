/*
	UART_IMX6Q.H
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Code to control the Freescale i.MX6Q Universal Asynchronous Receiver/Transmitter (UART).
	The UART is discussed in Chapter 63 (pages 5107-5182 of "i.MX 6Dual/6Quad Applications
	Processor Reference Manual, Rev. 0, 11/2012". The primary concern here is to provide a
	debug port during development.  The Octopus board has no external UART port, but it is
	brought to a connector (shared with the JTAG)
*/
#ifndef UART_IMX6Q_H_
#define UART_IMX6Q_H_

#include <stdint.h>
#include "debug.h"

/*
	class ATOSE_UART_IMX6Q
	----------------------
*/
class ATOSE_uart_imx6q : public ATOSE_debug
{
private:
	static const uint32_t baud_rate = 115200;				// as fast as we can reasonable go
	static const uint32_t port = 2;							// UART 2 is the "console" UART on the SABRE Lite board. UART 1 is the "other" UART on the Y-cable
	static const uint32_t PLL3_FREQUENCY = 80000000;		// Phase Lock Loop 3 runs at 80MHz

public:
	ATOSE_uart_imx6q();
	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes);			// to put a single character write(&c,1);
} ;

#endif

