/*
	IMX6Q_CLOCK.C
	-------------
	Code to generate a simple u-second timer
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsepit.h"


/*
	Default baud rate is 115200 and we'll talk down UARR-2 as that's the
	console on the SABRE Lite board
*/
#define BAUD_RATE 115200
#define DEFAULT_UART 2		/* can be either 2 (SABRE Lite "console") or 1 (the other UART) */
#define PLL3_FREQUENCY 80000000			// the frequency of Phase Lock Loop 3

#define DEFAULT_TIMER 1				// the i.MX6Q has 2 Enhanced Periodic Interrupt Timer (EPIT).

/*
	SERIAL_INIT()
	-------------
*/
void serial_init(void)
{
/*
   Enable Clocks
*/
*((uint32_t *)0x020C407C) = 0x0F0000C3;	// CCM Clock Gating Register 5 (CCM_CCGR5) (inc UART clock)

/*
   Enable Pads
*/
#if (DEFAULT_UART == 1)
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(SD3_DATA6_ALT1));
#elif (DEFAULT_UART == 2)
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA27_ALT4));

#else
	#error "Only UART 1 and 2 are supported"
#endif

/*
	Now on to the UART 8 bits, 1 stop bit, no parity, software flow control
*/
HW_UART_UCR1(DEFAULT_UART).U = 0;
HW_UART_UCR2(DEFAULT_UART).U = 0;
while (HW_UART_UCR2(DEFAULT_UART).B.SRST == 0)
	;	// nothing

HW_UART_UCR1(DEFAULT_UART).B.UARTEN = 1;
HW_UART_UCR3(DEFAULT_UART).B.RXDMUXSEL = 1;
HW_UART_UCR2_WR(DEFAULT_UART, BM_UART_UCR2_WS | BM_UART_UCR2_IRTS | BM_UART_UCR2_RXEN | BM_UART_UCR2_TXEN | BM_UART_UCR2_SRST);
HW_UART_UFCR(DEFAULT_UART).B.RFDIV = 0x04;		/* divide input clock by 2 */
HW_UART_UBIR(DEFAULT_UART).U = 0x0F;
HW_UART_UBMR(DEFAULT_UART).U = (PLL3_FREQUENCY / (HW_CCM_CSCDR1.B.UART_CLK_PODF + 1)) / (2 * BAUD_RATE);		// UBMR should be 0x015B once set
}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char value)
{
HW_UART_UTXD(DEFAULT_UART).U = value;
while (HW_UART_UTS(DEFAULT_UART).B.TXEMPTY == 0)
	; // do nothing
}

/*
	DEBUG_PUTS()
	------------
*/
void debug_puts(const char *string)
{
while (*string != 0)
	debug_putc(*string++);
}

/*
	EPIT_INIT()
	-----------
*/
void epit_init(void)
{
uint32_t speed_in_Hz[] = {528000000, 396000000, 352000000, 198000000, 594000000};
uint32_t frequency;

HW_CCM_CCGR1.B.CG6 = 0x03;			// turn on the clock

/*
	Software reset the subsystem
*/
HW_EPIT_CR_WR(DEFAULT_TIMER, BM_EPIT_CR_SWR);
while ((HW_EPIT_CR(DEFAULT_TIMER).B.SWR) != 0)
	;	// nothing

/*
	Configure the timer:
		Use the peripheral clock
		Set and forget mode
		Immediate load starting with the value in the load register
*/
frequency = speed_in_Hz[HW_CCM_CBCMR.B.PRE_PERIPH_CLK_SEL] / (HW_CCM_CBCDR.B.AHB_PODF + 1) / (HW_CCM_CBCDR.B.IPG_PODF + 1);
HW_EPIT_CR_WR(DEFAULT_TIMER, BF_EPIT_CR_CLKSRC(1) | BF_EPIT_CR_PRESCALAR((frequency / 1000000) - 1) | BM_EPIT_CR_RLD | BM_EPIT_CR_IOVW | BM_EPIT_CR_ENMOD);
}

/*
	EPIT_START()
	------------
*/
void epit_start(uint32_t time_in_us)
{
/*
	Store the count value in the load register (which then gets immediately loaded into the timer
*/
HW_EPIT_LR_WR(DEFAULT_TIMER, time_in_us);

/*
	Clear the status register so that we can watch it clock over
*/
HW_EPIT_SR_SET(DEFAULT_TIMER, BM_EPIT_SR_OCIF);

/*
	Start the timer
*/
HW_EPIT_CR_SET(DEFAULT_TIMER, BM_EPIT_CR_EN);
}

/*
	EPIT_STOP()
	-----------
*/
void epit_stop(void)
{
HW_EPIT_CR_CLR(DEFAULT_TIMER, BM_EPIT_CR_EN);
}

/*
	EPIT_DELAY_US()
	---------------
*/
void epit_delay_us(uint32_t usecs)
{
epit_start(usecs);
while (HW_EPIT_SR_RD(DEFAULT_TIMER) == 0)
	;	// nothing (i.e. wait)
epit_stop();
}

/*
	MAIN()
	------
*/
int main(void)
{
serial_init();
debug_puts("Serial Init...done\r\n");

debug_puts("EPIT Init...");
epit_init();
debug_puts("done\r\n");

debug_puts("Wait a second...");
epit_delay_us(1000000);
debug_puts("done\r\n");

debug_puts("Wait 10 seconds...");
epit_delay_us(10000000);
debug_puts("done\r\n");

return 0;
}
