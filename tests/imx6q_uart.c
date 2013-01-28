/*
	HELLO.C
	-------
*/
#include <stdint.h>
#include <string.h>

#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"

#define BAUD_RATE 115200
#define DEFAULT_UART 2

#define PLL3_FREQUENCY 80000000

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char value)
{
HW_UART_UTXD(DEFAULT_UART).U = value;

/*
	Make sure it was sent
*/
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
	{
	debug_putc(*string);
	string++;
	}
}

/*
	SERIAL_INIT()
	-------------
*/
void serial_init(void)
{
uint32_t clock;

/*
	Disable and soft reset the UART then wait for it to come up
*/
HW_UART_UCR1(DEFAULT_UART).U = 0;						// disable the UART
HW_UART_UCR2(DEFAULT_UART).U = 0;						// software reset (SRST)
while (HW_UART_UCR2(DEFAULT_UART).B.SRST == 0)
	;	// nothing

/*
	Enable the UART
*/
HW_UART_UCR1(DEFAULT_UART).B.UARTEN = 1;

/*
	8 bits, 1 stop bit, no parity, No hardware flow control
*/
/*                          8-bits   ignore RT   enable RX   Enable TX   don't reset */
HW_UART_UCR2_WR(DEFAULT_UART, BM_UART_UCR2_WS | BM_UART_UCR2_IRTS | BM_UART_UCR2_RXEN | BM_UART_UCR2_TXEN | BM_UART_UCR2_SRST);

/*
	RXDMUXSEL (must be on)
	DCD turned on
	RI is turned on
*/
HW_UART_UCR3_WR(DEFAULT_UART, BM_UART_UCR3_DSR | BM_UART_UCR3_DCD | BM_UART_UCR3_RI | BM_UART_UCR3_RXDMUXSEL);

/*
	CTSTL (32 character CTS triger)
*/
HW_UART_UCR4(DEFAULT_UART).B.CTSTL = 32;

/*
	Set the board rate
*/

/*
	Divide the clock by 2
	go into DCE mode (the i.MX6Q is the "computer end")
*/
HW_UART_UFCR(DEFAULT_UART).B.RFDIV = 0x04;		/* divide input clock by 2 */

/*
	Binary Rate Multiplier Numerator = 0x0F
*/
HW_UART_UBIR(DEFAULT_UART).U = 0x0F;

/*
	Binary Rate Multipier Denominator

	The "Module Clock" is the UART_CLK which comes from CCM.
	The "Peripheral Clock" is the IPG_CLK which comes from CCM.

	PLL3 runs at 80MHz by default
	PLL3 -> CDCDR1:uart_clk_podf (6 bit divider) -> UART_CLK_ROOT
*/
clock = PLL3_FREQUENCY / HW_CCM_CSCDR1.B.UART_CLK_PODF;
HW_UART_UBMR(DEFAULT_UART).U = clock / (2 * BAUD_RATE);		// UBMR should be 0x015B once set
}

/*
	IMX_IOMUX_V3_SETUP_PAD()
	------------------------
*/
static void *base = (void *)IOMUXC_BASE_ADDR;
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))
#define __raw_writel(v,a)	__arch_putl(v,a)
int imx_iomux_v3_setup_pad(iomux_v3_cfg_t pad)
{
u32 mux_ctrl_ofs = (pad & MUX_CTRL_OFS_MASK) >> MUX_CTRL_OFS_SHIFT;
u32 mux_mode = (pad & MUX_MODE_MASK) >> MUX_MODE_SHIFT;
u32 sel_input_ofs = (pad & MUX_SEL_INPUT_OFS_MASK) >> MUX_SEL_INPUT_OFS_SHIFT;
u32 sel_input = (pad & MUX_SEL_INPUT_MASK) >> MUX_SEL_INPUT_SHIFT;
u32 pad_ctrl_ofs = (pad & MUX_PAD_CTRL_OFS_MASK) >> MUX_PAD_CTRL_OFS_SHIFT;
u32 pad_ctrl = (pad & MUX_PAD_CTRL_MASK) >> MUX_PAD_CTRL_SHIFT;

if (mux_ctrl_ofs)
	__raw_writel(mux_mode, base + mux_ctrl_ofs);

if (sel_input_ofs)
	__raw_writel(sel_input, base + sel_input_ofs);

if (!(pad_ctrl & NO_PAD_CTRL) && pad_ctrl_ofs)
	__raw_writel(pad_ctrl, base + pad_ctrl_ofs);

return 0;
}

/*
	IMX_IOMUX_V3_SETUP_MULTIPLE_PADS()
	----------------------------------
*/
int imx_iomux_v3_setup_multiple_pads(iomux_v3_cfg_t const *pad_list, unsigned count)
{
iomux_v3_cfg_t const *p = pad_list;
unsigned i;
int ret;

for (i = 0; i < count; i++)
	{
	ret = imx_iomux_v3_setup_pad(*p);
	if (ret)
		return ret;
	p++;
	}

return 0;
}

/*
	PAD_CONTROL()
	-------------
*/
void pad_control(void)
{
#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

iomux_v3_cfg_t const uart1_pads[] =
{
MX6Q_PAD_SD3_DAT6__UART1_RXD | MUX_PAD_CTRL(UART_PAD_CTRL), 
MX6Q_PAD_SD3_DAT7__UART1_TXD | MUX_PAD_CTRL(UART_PAD_CTRL)
} ;

iomux_v3_cfg_t const uart2_pads[] = 
{
MX6Q_PAD_EIM_D26__UART2_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
MX6Q_PAD_EIM_D27__UART2_RXD | MUX_PAD_CTRL(UART_PAD_CTRL)
} ;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

/*
	CONFIGURE_SABERLITE()
	---------------------
*/
void configure_saberlite(void)
{
*((uint32_t *)0x020E05A8) = 0x00000030;
*((uint32_t *)0x020E05B0) = 0x00000030;
*((uint32_t *)0x020E0524) = 0x00000030;
*((uint32_t *)0x020E051C) = 0x00000030;
*((uint32_t *)0x020E0518) = 0x00000030;
*((uint32_t *)0x020E050C) = 0x00000030;
*((uint32_t *)0x020E05B8) = 0x00000030;
*((uint32_t *)0x020E05C0) = 0x00000030;
*((uint32_t *)0x020E05AC) = 0x00020030;
*((uint32_t *)0x020E05B4) = 0x00020030;
*((uint32_t *)0x020E0528) = 0x00020030;
*((uint32_t *)0x020E0520) = 0x00020030;
*((uint32_t *)0x020E0514) = 0x00020030;
*((uint32_t *)0x020E0510) = 0x00020030;
*((uint32_t *)0x020E05BC) = 0x00020030;
*((uint32_t *)0x020E05C4) = 0x00020030;
*((uint32_t *)0x020E056C) = 0x00020030;
*((uint32_t *)0x020E0578) = 0x00020030;
*((uint32_t *)0x020E0588) = 0x00020030;
*((uint32_t *)0x020E0594) = 0x00020030;
*((uint32_t *)0x020E057C) = 0x00020030;
*((uint32_t *)0x020E0590) = 0x00003000;
*((uint32_t *)0x020E0598) = 0x00003000;
*((uint32_t *)0x020E058C) = 0x00000000;
*((uint32_t *)0x020E059C) = 0x00003030;
*((uint32_t *)0x020E05A0) = 0x00003030;
*((uint32_t *)0x020E0784) = 0x00000030;
*((uint32_t *)0x020E0788) = 0x00000030;
*((uint32_t *)0x020E0794) = 0x00000030;
*((uint32_t *)0x020E079C) = 0x00000030;
*((uint32_t *)0x020E07A0) = 0x00000030;
*((uint32_t *)0x020E07A4) = 0x00000030;
*((uint32_t *)0x020E07A8) = 0x00000030;
*((uint32_t *)0x020E0748) = 0x00000030;
*((uint32_t *)0x020E074C) = 0x00000030;
*((uint32_t *)0x020E0750) = 0x00020000;
*((uint32_t *)0x020E0758) = 0x00000000;
*((uint32_t *)0x020E0774) = 0x00020000;
*((uint32_t *)0x020E078C) = 0x00000030;
*((uint32_t *)0x020E0798) = 0x000C0000;
*((uint32_t *)0x021B081C) = 0x33333333;
*((uint32_t *)0x021B0820) = 0x33333333;
*((uint32_t *)0x021B0824) = 0x33333333;
*((uint32_t *)0x021B0828) = 0x33333333;
*((uint32_t *)0x021B481C) = 0x33333333;
*((uint32_t *)0x021B4820) = 0x33333333;
*((uint32_t *)0x021B4824) = 0x33333333;
*((uint32_t *)0x021B4828) = 0x33333333;
*((uint32_t *)0x021B0018) = 0x00081740;
*((uint32_t *)0x021B001C) = 0x00008000;
*((uint32_t *)0x021B000C) = 0x555A7975;
*((uint32_t *)0x021B0010) = 0xFF538E64;
*((uint32_t *)0x021B0014) = 0x01FF00DB;
*((uint32_t *)0x021B002C) = 0x000026D2;
*((uint32_t *)0x021B0030) = 0x005B0E21;
*((uint32_t *)0x021B0008) = 0x09444040;
*((uint32_t *)0x021B0004) = 0x00025576;
*((uint32_t *)0x021B0040) = 0x00000027;
*((uint32_t *)0x021B0000) = 0x831A0000;
*((uint32_t *)0x021B001C) = 0x04088032;
*((uint32_t *)0x021B001C) = 0x0408803A;
*((uint32_t *)0x021B001C) = 0x00008033;
*((uint32_t *)0x021B001C) = 0x0000803B;
*((uint32_t *)0x021B001C) = 0x00428031;
*((uint32_t *)0x021B001C) = 0x00428039;
*((uint32_t *)0x021B001C) = 0x09408030;
*((uint32_t *)0x021B001C) = 0x09408038;
*((uint32_t *)0x021B001C) = 0x04008040;
*((uint32_t *)0x021B001C) = 0x04008048;
*((uint32_t *)0x021B0800) = 0xA1380003;
*((uint32_t *)0x021B4800) = 0xA1380003;
*((uint32_t *)0x021B0020) = 0x00005800;
*((uint32_t *)0x021B0818) = 0x00022227;
*((uint32_t *)0x021B4818) = 0x00022227;
*((uint32_t *)0x021B083C) = 0x434B0350;
*((uint32_t *)0x021B0840) = 0x034C0359;
*((uint32_t *)0x021B483C) = 0x434B0350;
*((uint32_t *)0x021B4840) = 0x03650348;
*((uint32_t *)0x021B0848) = 0x4436383B;
*((uint32_t *)0x021B4848) = 0x39393341;
*((uint32_t *)0x021B0850) = 0x35373933;
*((uint32_t *)0x021B4850) = 0x48254A36;
*((uint32_t *)0x021B080C) = 0x001F001F;
*((uint32_t *)0x021B0810) = 0x001F001F;
*((uint32_t *)0x021B480C) = 0x00440044;
*((uint32_t *)0x021B4810) = 0x00440044;
*((uint32_t *)0x021B08B8) = 0x00000800;
*((uint32_t *)0x021B48B8) = 0x00000800;
*((uint32_t *)0x021B001C) = 0x00000000;
*((uint32_t *)0x021B0404) = 0x00011006;
*((uint32_t *)0x020C4068) = 0x00C03F3F;
*((uint32_t *)0x020C406C) = 0x0030FC03;
*((uint32_t *)0x020C4070) = 0x0FFFC000;
*((uint32_t *)0x020C4074) = 0x3FF00000;
*((uint32_t *)0x020C4078) = 0x00FFF300;
*((uint32_t *)0x020C407C) = 0x0F0000C3;
*((uint32_t *)0x020C4080) = 0x000003FF;
*((uint32_t *)0x020E0010) = 0xF00000CF;
*((uint32_t *)0x020E0018) = 0x007F007F;
*((uint32_t *)0x020E001C) = 0x007F007F;
}

/*
	MAIN()
	------
*/
int main(void)
{
configure_saberlite();
pad_control();
serial_init();

debug_puts("Hello, world!\r\n");

return 0;
}
