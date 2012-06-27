/*
	IO_DEBUG_IMX233.C
	-----------------
*/
#include "ascii_str.h"
#include "io_debug_imx233.h"

#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"


/*
	ATOSE_IO_DEBUG_IMX233::ENABLE()
	-------------------------------
*/
void ATOSE_IO_debug_imx233::enable(void)
{
/*
	Enable interrupt on recieve of each character
*/
HW_UARTDBGLCR_H_WR(BF_UARTDBGLCR_H_WLEN(3));											// set N,8,1 (and turn off the FIFOs)
HW_UARTDBGIFLS_WR(0);																	// Interrupt on non-empty FIFOs (turn off buffering)
HW_UARTDBGIMSC_SET(BM_UARTDBGIMSC_RXIM);												// interrupt on recieve (RXIM)
HW_UARTDBGCR_SET(BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE | BM_UARTDBGCR_UARTEN);			// turn on UART and send and recieve buffers
}

/*
	ATOSE_IO_DEBUG_IMX233::DISABLE()
	--------------------------------
*/
void ATOSE_IO_debug_imx233::disable(void)
{
HW_UARTDBGCR_CLR(BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE | BM_UARTDBGCR_UARTEN);			// turn off UART and send and recieve buffers
}

/*
	ATOSE_IO_DEBUG_IMX233::ACKNOWLEDGE()
	------------------------------------
*/
void ATOSE_IO_debug_imx233::acknowledge(void)
{
uint8_t got;

/*
	Clear all the interrupt bits
*/
HW_UARTDBGICR_WR(0x7FF);

/*
	shove the key press into the I/O buffers
*/
got = HW_UARTDBGDR_RD();
push(&got);
}

/*
	ATOSE_IO_DEBUG_IMX233::PUSH()
	-----------------------------
*/
void ATOSE_IO_debug_imx233::push(uint8_t *byte)
{
buffer.write(*byte);
}

/*
	ATOSE_IO_DEBUG_IMX233::WRITE()
	------------------------------
*/
uint32_t ATOSE_IO_debug_imx233::write(const uint8_t *buffer, uint32_t bytes)
{
uint32_t loop, current;

for (current = 0; current < bytes; current++)
	{
	int loop = 0;

	while (HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF)
		if (++loop > 10000)
			break;

	HW_UARTDBGDR_WR(buffer[current]);
	}

return bytes;
}

/*
	ATOSE_IO_DEBUG_IMX233::READ()
	-----------------------------
*/
uint32_t ATOSE_IO_debug_imx233::read(uint8_t *into, uint32_t  bytes)
{
uint32_t which;

for (which = 0; which < bytes; which++)
	{
	if (buffer.is_empty())
		return which;
	*into++ = buffer.read();
	}

return bytes;
}
