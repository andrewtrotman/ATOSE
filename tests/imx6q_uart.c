/*
	HELLO.C
	-------
*/
#include <stdint.h>
#include <string.h>

#include "asm/arch-mx6/imx-regs.h"
#include "asm/arch-mx6/crm_regs.h"
#include "asm/arch-mx6/mx6x_pins.h"


#define __REG(x)     (*((volatile u32 *)(x)))

#define UART1_PHYS 0x02020000
#define UART2_PHYS 0x021E8000

#define CONFIG_BAUDRATE 115200
#define ATOSE_UART UART2_PHYS

/* Register definitions */
#define URXD  0x0  /* Receiver Register */
#define UTXD  0x40 /* Transmitter Register */
#define UCR1  0x80 /* Control Register 1 */
#define UCR2  0x84 /* Control Register 2 */
#define UCR3  0x88 /* Control Register 3 */
#define UCR4  0x8c /* Control Register 4 */
#define UFCR  0x90 /* FIFO Control Register */
#define USR1  0x94 /* Status Register 1 */
#define USR2  0x98 /* Status Register 2 */
#define UESC  0x9c /* Escape Character Register */
#define UTIM  0xa0 /* Escape Timer Register */
#define UBIR  0xa4 /* BRM Incremental Register */
#define UBMR  0xa8 /* BRM Modulator Register */
#define UBRC  0xac /* Baud Rate Count Register */
#define UTS   0xb4 /* UART Test Register (mx31) */
#define UMCR  0xb8 /* UMCR RS-485 Mode Control Register */

/* UART Control Register Bit Fields.*/
#define  URXD_CHARRDY    (1<<15)
#define  URXD_ERR        (1<<14)
#define  URXD_OVRRUN     (1<<13)
#define  URXD_FRMERR     (1<<12)
#define  URXD_BRK        (1<<11)
#define  URXD_PRERR      (1<<10)
#define  URXD_RX_DATA    (0xFF)
#define  UCR1_ADEN       (1<<15) /* Auto dectect interrupt */
#define  UCR1_ADBR       (1<<14) /* Auto detect baud rate */
#define  UCR1_TRDYEN     (1<<13) /* Transmitter ready interrupt enable */
#define  UCR1_IDEN       (1<<12) /* Idle condition interrupt */
#define  UCR1_RRDYEN     (1<<9)	 /* Recv ready interrupt enable */
#define  UCR1_RDMAEN     (1<<8)	 /* Recv ready DMA enable */
#define  UCR1_IREN       (1<<7)	 /* Infrared interface enable */
#define  UCR1_TXMPTYEN   (1<<6)	 /* Transimitter empty interrupt enable */
#define  UCR1_RTSDEN     (1<<5)	 /* RTS delta interrupt enable */
#define  UCR1_SNDBRK     (1<<4)	 /* Send break */
#define  UCR1_TDMAEN     (1<<3)	 /* Transmitter ready DMA enable */
#define  UCR1_UARTCLKEN  (1<<2)	 /* UART clock enabled */
#define  UCR1_DOZE       (1<<1)	 /* Doze */
#define  UCR1_UARTEN     (1<<0)	 /* UART enabled */
#define  UCR2_ESCI	 (1<<15) /* Escape seq interrupt enable */
#define  UCR2_IRTS	 (1<<14) /* Ignore RTS pin */
#define  UCR2_CTSC	 (1<<13) /* CTS pin control */
#define  UCR2_CTS        (1<<12) /* Clear to send */
#define  UCR2_ESCEN      (1<<11) /* Escape enable */
#define  UCR2_PREN       (1<<8)  /* Parity enable */
#define  UCR2_PROE       (1<<7)  /* Parity odd/even */
#define  UCR2_STPB       (1<<6)	 /* Stop */
#define  UCR2_WS         (1<<5)	 /* Word size */
#define  UCR2_RTSEN      (1<<4)	 /* Request to send interrupt enable */
#define  UCR2_TXEN       (1<<2)	 /* Transmitter enabled */
#define  UCR2_RXEN       (1<<1)	 /* Receiver enabled */
#define  UCR2_SRST	 (1<<0)	 /* SW reset */
#define  UCR3_DTREN	 (1<<13) /* DTR interrupt enable */
#define  UCR3_PARERREN   (1<<12) /* Parity enable */
#define  UCR3_FRAERREN   (1<<11) /* Frame error interrupt enable */
#define  UCR3_DSR        (1<<10) /* Data set ready */
#define  UCR3_DCD        (1<<9)  /* Data carrier detect */
#define  UCR3_RI         (1<<8)  /* Ring indicator */
#define  UCR3_TIMEOUTEN  (1<<7)  /* Timeout interrupt enable */
#define  UCR3_RXDSEN	 (1<<6)  /* Receive status interrupt enable */
#define  UCR3_AIRINTEN   (1<<5)  /* Async IR wake interrupt enable */
#define  UCR3_AWAKEN	 (1<<4)  /* Async wake interrupt enable */
#define  UCR3_REF25	 (1<<3)  /* Ref freq 25 MHz */
#define  UCR3_REF30	 (1<<2)  /* Ref Freq 30 MHz */
#define  UCR3_INVT	 (1<<1)  /* Inverted Infrared transmission */
#define  UCR3_BPEN	 (1<<0)  /* Preset registers enable */
#define  UCR4_CTSTL_32   (32<<10) /* CTS trigger level (32 chars) */
#define  UCR4_INVR	 (1<<9)  /* Inverted infrared reception */
#define  UCR4_ENIRI	 (1<<8)  /* Serial infrared interrupt enable */
#define  UCR4_WKEN	 (1<<7)  /* Wake interrupt enable */
#define  UCR4_REF16	 (1<<6)  /* Ref freq 16 MHz */
#define  UCR4_IRSC	 (1<<5)  /* IR special case */
#define  UCR4_TCEN	 (1<<3)  /* Transmit complete interrupt enable */
#define  UCR4_BKEN	 (1<<2)  /* Break condition interrupt enable */
#define  UCR4_OREN	 (1<<1)  /* Receiver overrun interrupt enable */
#define  UCR4_DREN	 (1<<0)  /* Recv data ready interrupt enable */
#define  UFCR_RXTL_SHF   0       /* Receiver trigger level shift */
#define  UFCR_RFDIV      (7<<7)  /* Reference freq divider mask */
#define  UFCR_TXTL_SHF   10      /* Transmitter trigger level shift */
#define  USR1_PARITYERR  (1<<15) /* Parity error interrupt flag */
#define  USR1_RTSS	 (1<<14) /* RTS pin status */
#define  USR1_TRDY	 (1<<13) /* Transmitter ready interrupt/dma flag */
#define  USR1_RTSD	 (1<<12) /* RTS delta */
#define  USR1_ESCF	 (1<<11) /* Escape seq interrupt flag */
#define  USR1_FRAMERR    (1<<10) /* Frame error interrupt flag */
#define  USR1_RRDY       (1<<9)	 /* Receiver ready interrupt/dma flag */
#define  USR1_TIMEOUT    (1<<7)	 /* Receive timeout interrupt status */
#define  USR1_RXDS	 (1<<6)	 /* Receiver idle interrupt flag */
#define  USR1_AIRINT	 (1<<5)	 /* Async IR wake interrupt flag */
#define  USR1_AWAKE	 (1<<4)	 /* Aysnc wake interrupt flag */
#define  USR2_ADET	 (1<<15) /* Auto baud rate detect complete */
#define  USR2_TXFE	 (1<<14) /* Transmit buffer FIFO empty */
#define  USR2_DTRF	 (1<<13) /* DTR edge interrupt flag */
#define  USR2_IDLE	 (1<<12) /* Idle condition */
#define  USR2_IRINT	 (1<<8)	 /* Serial infrared interrupt flag */
#define  USR2_WAKE	 (1<<7)	 /* Wake */
#define  USR2_RTSF	 (1<<4)	 /* RTS edge interrupt flag */
#define  USR2_TXDC	 (1<<3)	 /* Transmitter complete */
#define  USR2_BRCD	 (1<<2)	 /* Break condition */
#define  USR2_ORE        (1<<1)	 /* Overrun error */
#define  USR2_RDR        (1<<0)	 /* Recv data ready */
#define  UTS_FRCPERR	 (1<<13) /* Force parity error */
#define  UTS_LOOP        (1<<12) /* Loop tx and rx */
#define  UTS_TXEMPTY	 (1<<6)	 /* TxFIFO empty */
#define  UTS_RXEMPTY	 (1<<5)	 /* RxFIFO empty */
#define  UTS_TXFULL	 (1<<4)	 /* TxFIFO full */
#define  UTS_RXFULL	 (1<<3)	 /* RxFIFO full */
#define  UTS_SOFTRST	 (1<<0)	 /* Software reset */

struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

/*
	IMX_GET_UART_CLK()
	------------------
	The "Module Clock" is the UART_CLK which comes from CCM.
	The "Peripheral Clock" is the IPG_CLK which comes from CCM.

	PLL3 runs at 80MHz by default
	PLL3 -> CDCDR1:uart_clk_podf (6 bit divider) -> UART_CLK_ROOT
*/
static u32 imx_get_uart_clk(void)
{
u32 reg, uart_podf;

reg = __REG(&imx_ccm->cscdr1);
reg &= MXC_CCM_CSCDR1_UART_CLK_PODF_MASK;
uart_podf = reg >> MXC_CCM_CSCDR1_UART_CLK_PODF_OFFSET;

return PLL3_80M / (uart_podf + 1);
}

/*
	MXC_SERIAL_SETBRG()
	-------------------
*/
static void mxc_serial_setbrg(void)
{
u32 clk = imx_get_uart_clk();

/*
	Divide the clock by 
	go into DCE mode (the i.MX6Q is the "computer end")
*/
__REG(ATOSE_UART + UFCR) = 4 << 7; /* divide input clock by 2 */		// SAME

/*
	Binary Rate Multiplier Numerator = 0x0F
*/
__REG(ATOSE_UART + UBIR) = 0xf;										// SAME

/*
	Binary Rate Multipier Denominator
*/
__REG(ATOSE_UART + UBMR) = clk / (2 * CONFIG_BAUDRATE);
//	the value of __REG(ATOSE_UART + UBMR)  should be 0x15B;
}

/*
	MXC_SERIAL_GETC()
	-----------------
*/
static int mxc_serial_getc(void)
{
while (__REG(ATOSE_UART + UTS) & UTS_RXEMPTY)
	; // nothing;

return (__REG(ATOSE_UART + URXD) & URXD_RX_DATA); /* mask out status from upper word */
}

/*
	MXC_SERIAL_PUTC()
	-----------------
*/
static void mxc_serial_putc(const char c)
{
__REG(ATOSE_UART + UTXD) = c;

/* wait for transmitter to be ready */
while (!(__REG(ATOSE_UART + UTS) & UTS_TXEMPTY))
	; // nothing;
}

/*
	MXC_SERIAL_PUTS()
	-----------------
*/
static void mxc_serial_puts(const char *s)
{
while (*s)
	{
	mxc_serial_putc(*s);
	s++;
	}
}

/*
	MXC_SERIAL_TSTC()
	-----------------
*/
static int mxc_serial_tstc(void)
{
/* If receive fifo is empty, return false */
if (__REG(ATOSE_UART + UTS) & UTS_RXEMPTY)
	return 0;

return 1;
}

/*
	ORIGINAL_MXC_SERIAL_INIT()
	--------------------------
	This is how it was when I got it fron Nick
*/
static int original_mxc_serial_init(void)
{
__REG(ATOSE_UART + UCR1) = 0x0;
__REG(ATOSE_UART + UCR2) = 0x0;

while (!(__REG(ATOSE_UART + UCR2) & UCR2_SRST))
	;	// nothing

__REG(ATOSE_UART + UCR3) = 0x0704;
__REG(ATOSE_UART + UCR4) = 0x8000;
__REG(ATOSE_UART + UESC) = 0x002b;
__REG(ATOSE_UART + UTIM) = 0x0;

__REG(ATOSE_UART + UTS) = 0x0;

mxc_serial_setbrg();

__REG(ATOSE_UART + UCR2) = UCR2_WS | UCR2_IRTS | UCR2_RXEN | UCR2_TXEN | UCR2_SRST;

__REG(ATOSE_UART + UCR1) = UCR1_UARTEN;

return 0;
}

/*
	MXC_SERIAL_INIT()
	-----------------
*/
static int mxc_serial_init(void)
{
/*
	Disable and soft reset the UART then wait for it to come up
*/
__REG(ATOSE_UART + UCR1) = 0x0;						// disable the UART
__REG(ATOSE_UART + UCR2) = 0x0;						// software reset (SRST)
while (!(__REG(ATOSE_UART + UCR2) & UCR2_SRST))		// wait until SRST returns true
	;	// nothing

/*
	Enable the UART
*/
__REG(ATOSE_UART + UCR1) = UCR1_UARTEN;	// SAME

/*
	8 bits, 1 stop bit, no parity, No hardware flow control
*/
/*                          8-bits   ignore RT   enable RX   Enable TX   don't reset */
__REG(ATOSE_UART + UCR2) = UCR2_WS | UCR2_IRTS | UCR2_RXEN | UCR2_TXEN | UCR2_SRST;		// SAME

/*
	RXDMUXSEL (must be on)
	DCD turned on
	RI is turned on
*/
__REG(ATOSE_UART + UCR3) = 0x0704;			// SAME

/*
	CTSTL (32 character CTS triger)
*/
__REG(ATOSE_UART + UCR4) = 0x8000;			// SAME

mxc_serial_setbrg();

/*
	set the "excape" character to '+'
*/
__REG(ATOSE_UART + UESC) = 0x002b;			// SAME

/*
	2ms timeout between "escape" characters
*/
__REG(ATOSE_UART + UTIM) = 0x0;			// SAME

/*
	disable UART "debug" stuff
*/
__REG(ATOSE_UART + UTS) = 0x0;				// SAME (for said bits)

/*
	Make sure we're on
*/
__REG(ATOSE_UART + UCR1) = UCR1_UARTEN;	// SAME

/*
	Make sure we're in RS232 mode
*/
__REG(ATOSE_UART + UMCR) = 0x00;			// SAME

return 0;
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
char *now = "TIME:" __TIME__;
unsigned char *text = (unsigned char *)0x910000;
long len = strlen(now);

memcpy(text, now, len);
text += len;

configure_saberlite();

pad_control();

mxc_serial_init();

mxc_serial_puts("Hello, world!\r\n");
mxc_serial_puts("Hello, world!\r\n");
mxc_serial_puts("Hello, world!\r\n");
mxc_serial_puts("Hello, world!\r\n");
mxc_serial_puts("Hello, world!\r\n");

memcpy(text, now, len);

return 0;
}
