/*
	IMX6Q_INTERRUPT.C
	-----------------
*/
#include <stdint.h>


#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsgpt.h"

#define BAUD_RATE 115200
#define DEFAULT_UART 2		/* can be either 2 (SABRE Lite "console") or 1 (the other UART) */
#define PLL3_FREQUENCY 80000000

volatile uint32_t count = 32;

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
#elif (DEFAULT_UART == 2)
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
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
void debug_puts(char *string)
{
while (*string != '\0')
	debug_putc(*string++);
}

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(int data)
{
int i = 0;
char c;

for (i = sizeof(int)*2-1; i >= 0; i--)
	{
	c = data>>(i*4);
	c &= 0xf;
	if (c > 9)
		debug_putc(c-10+'A');
	else
		debug_putc(c+'0');
	}
}

/*
   TIMER_INIT()
   ------------
*/
void timer_init(void)
{
HW_GPT_CR.B.EN = 0;
HW_GPT_IR.U = 0;
HW_GPT_CR.B.OM1 = HW_GPT_CR.B.OM2 = HW_GPT_CR.B.OM3 = 0;
HW_GPT_CR.B.IM1 = HW_GPT_CR.B.IM2 = 0;
HW_GPT_CR.B.CLKSRC = 0x07;      // use the 24MHz crystal
HW_GPT_CR.B.SWR = 0;
while (HW_GPT_CR.B.SWR != 0)
   ;/* nothing */
HW_GPT_SR.U = 0;
HW_GPT_CR.B.ENMOD = 1;
HW_GPT_CR.B.EN = 1;
}

/*
   MAIN()
   ------
*/
int main(void)
{
long x;
uint32_t count, rollover;

serial_init();
debug_puts("\r\nStart:" __TIME__ "\r\n");
timer_init();
debug_puts("Timer initilised\r\n");


HW_GPT_OCR1.U = 0x100;     // fun until the GPT gets to 0x100
HW_GPT_CR.B.FRR = 0;    // restart mode (periodic interrupt mode)

for (x = 0; x < 10; x++)
   {
   count = HW_GPT_CNT_RD();
   rollover = HW_GPT_SR_RD();

   debug_puts("GPT_CNT:");
   debug_print_hex(count);
   debug_puts(" ");

   debug_puts("GPT_SR:");
   debug_print_hex(rollover);
   debug_puts(" ");
   if (rollover != 0)
      HW_GPT_SR.B.OF1 = 1;       // clear the roll_over bit

   debug_puts("\r\n");
   }

return 0;
}
