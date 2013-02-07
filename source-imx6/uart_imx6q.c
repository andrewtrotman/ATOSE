/*
	UART_IMX6Q.C
	------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Code to control the Freescale i.MX6Q Universal Asynchronous Receiver/Transmitter (UART).
	The UART is discussed in Chapter 63 (pages 5107-5182 of "i.MX 6Dual/6Quad Applications
	Processor Reference Manual, Rev. 0, 11/2012". The primary concern here is to provide a
	debug port during development.  The Octopus board has no external UART port, but it is
	brought to a connector (shared with the JTAG)
*/
#include <string.h>
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "uart_imx6q.h"

/*
	ATOSE_UART_IMX6Q::ATOSE_UART_IMX6Q()
	------------------------------------
*/
ATOSE_uart_imx6q::ATOSE_uart_imx6q() : ATOSE_debug()
{
/*
	Enable the clock to the UART
*/
*((uint32_t *)0x020C407C) = 0x0F0000C3;	// CCM Clock Gating Register 5 (CCM_CCGR5) (inc UART clock)

/*
	Disable and soft reset the UART then wait for it to come up
*/
HW_UART_UCR1(port).U = 0;						// disable the UART
HW_UART_UCR2(port).U = 0;						// software reset (SRST)
while (HW_UART_UCR2(port).B.SRST == 0)
	;	// nothing

/*
	Enable the UART
	Enable RXDMUXSEL (must be on)
*/
HW_UART_UCR1(port).B.UARTEN = 1;
HW_UART_UCR3(port).B.RXDMUXSEL = 1;

/*
	8 bits, 1 stop bit, no parity,software flow control
*/
/*                                8-bits             ignore RTS         enable RX            enable TX         don't reset */
HW_UART_UCR2_WR(port, BM_UART_UCR2_WS | BM_UART_UCR2_IRTS | BM_UART_UCR2_RXEN | BM_UART_UCR2_TXEN | BM_UART_UCR2_SRST);

/*
	Set the board rate

	The "Module Clock" is the UART_CLK which comes from CCM.
	The "Peripheral Clock" is the IPG_CLK which comes from CCM.

	PLL3 runs at 80MHz by default
	PLL3 -> CDCDR1:uart_clk_podf (6 bit divider) -> UART_CLK_ROOT
*/

/*
	Divide the clock by 2
*/
HW_UART_UFCR(port).B.RFDIV = 0x04;		/* divide input clock by 2 */

/*
	Binary Rate Multiplier Numerator = 0x0F
*/
HW_UART_UBIR(port).U = 0x0F;

/*
	Binary Rate Multipier Denominator set based on the baud rate
*/
HW_UART_UBMR(port).U = (PLL3_FREQUENCY / (HW_CCM_CSCDR1.B.UART_CLK_PODF + 1)) / (2 * baud_rate);		// UBMR should be 0x015B once set

/*
	Finally, enable the pins
	This code was extracted from code automatically generated by the Freescale IOMUX tool for the i.MX6Q and is
	used to turn on the UART pads (i.e. external pins)
*/
if (port == 1)
	{
	/*
		On the SABRE Lite, UART-1 {the "other" UART) comes out at pins E13 and F13
	*/
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));


	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	}
else if (port == 2)
	{
	/*
		On the SABRE Lite UART-2 (the "console") comes out at pins E24 and E25
	*/
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));

	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	}
else
	{
	/*
		We don't currently support other than UART 1 and UART 2
	*/
	}
}

/*
	ATOSE_UART_IMX6Q::WRITE()
	-------------------------
*/
uint32_t ATOSE_uart_imx6q::write(uint8_t *buffer, uint32_t bytes)
{
while (*buffer != '\0')
	{
	/*
		Write to the serial port
	*/
	HW_UART_UTXD(port).U = *buffer;

	/*
		Make sure it was sent
	*/
	while (HW_UART_UTS(port).B.TXEMPTY == 0)
		; // do nothing

	/*
		Move on to the next character
	*/
	buffer++;
	}

return bytes;
}


