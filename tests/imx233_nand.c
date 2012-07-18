/*
	IMX233_NAND.C
	-------------
	Experiments with the I.MX233 GPMI
*/
#include <stdint.h>
#include "../source/registers.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsrtc.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsicoll.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspinctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsgpmi.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbh.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsclkctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbh.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsdigctl.h"

/*
	Interrupt Service Routines
*/
void __cs3_isr_undef(void){}
void __cs3_isr_pabort(void){}
void __cs3_isr_dabort(void){}
void __cs3_isr_reserved(void){}
void __cs3_isr_fiq(void){}
void ATOSE_isr_swi(void){}

/**
 * struct mxs_dma_cmd_bits - MXS DMA hardware command bits.
 *
 * This structure describes the in-memory layout of the command bits in a DMA
 * command. See the appropriate reference manual for a detailed description
 * of what these bits mean to the DMA hardware.
 */
struct mxs_dma_cmd_bits {
	unsigned int command:2;
#define NO_DMA_XFER	0x00
#define DMA_WRITE	0x01
#define DMA_READ	0x02
#define DMA_SENSE	0x03

	unsigned int chain:1;
	unsigned int irq:1;
	unsigned int nand_lock:1;
	unsigned int nand_wait_4_ready:1;
	unsigned int dec_sem:1;
	unsigned int wait4end:1;
	unsigned int halt_on_terminate:1;
	unsigned int terminate_flush:1;
	unsigned int resv2:2;
	unsigned int pio_words:4;
	unsigned int bytes:16;
};

/**
 * struct mxs_dma_cmd - MXS DMA hardware command.
 *
 * This structure describes the in-memory layout of an entire DMA command,
 * including space for the maximum number of PIO accesses. See the appropriate
 * reference manual for a detailed description of what these fields mean to the
 * DMA hardware.
 */
#define DMA_PIO_WORDS	15

#define NAND_DEVICE_ID_BYTE_COUNT  (6)

struct mxs_dma_cmd {
	unsigned long next;
	union {
		unsigned long data;
		struct mxs_dma_cmd_bits bits;
	} cmd;
	union {
		char *address;
		unsigned long alternate;
	};
	unsigned long pio_words[DMA_PIO_WORDS];
};

struct mxs_dma_cmd dma_queue[8];
struct mxs_dma_cmd *dma_queue_tail = dma_queue;

uint32_t gpmi_freq = 10; // MHz

/*
	NAND_SELECT_PINS()
	------------------
*/
void nand_select_pins(void)
{
uint32_t current_pin_state;

/*
	Tell the i.MX233 to use the pins for GPMI
*/
/*
	Turn on the 8-bit data bus and leave the other pins how they were
*/
current_pin_state = HW_PINCTRL_MUXSEL0_RD();
current_pin_state = current_pin_state & 0xFFFF0000;
HW_PINCTRL_MUXSEL0_WR(current_pin_state);
/*
	Turn on the control lines
*/
current_pin_state = HW_PINCTRL_MUXSEL1_RD();
current_pin_state = current_pin_state  & 0xFFF00000; //& 0xFFF03C30;
HW_PINCTRL_MUXSEL1_WR(current_pin_state);

current_pin_state = HW_PINCTRL_MUXSEL5_RD();
current_pin_state = current_pin_state & 0xFc3FFFFF;
HW_PINCTRL_MUXSEL5_WR(current_pin_state);

/*
	Tell the i.MX233 to use the right voltages and current
*/
/*
	Data bus at 4mA
*/
HW_PINCTRL_DRIVE0_WR(0x00000000);
/*
	drive WPN at 12mA and the other control lines at 4mA
*/
HW_PINCTRL_DRIVE2_WR(0x10000000);
/*
	RDN and WRN at 12mA
*/
HW_PINCTRL_DRIVE3_WR((HW_PINCTRL_DRIVE3_RD() & 0xFFFFFF00) | 0x22);
/*
	and the remaining conrol lines at 4mA
*/
HW_PINCTRL_DRIVE11_WR(HW_PINCTRL_DRIVE11_RD() & 0xFFF00FFF);

/*
	I see no way to affect the voltages on the lines (so I guess GMPI always runs at 3.3V)
*/
/*
	Now turn off the pull-up resisters (turn on the pad keepers)
*/

//HW_PINCTRL_PULL0_CLR(0x001800FF);
//HW_PINCTRL_PULL2_CLR(0x18000000);

/*
	We're done - yay!
*/
}

/*
	DELAY_US()
	----------
*/
void delay_us(unsigned long int us)
{
unsigned long int start = HW_DIGCTL_MICROSECONDS_RD();

while ((start + us) > HW_DIGCTL_MICROSECONDS_RD())
	/* nothing */ ;
}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char ch)
{
int loop = 0;

while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF)
	if (++loop > 10000)
		break;

/* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
HW_UARTDBGDR_WR(ch);
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
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex_byte(uint8_t data)
{
char *string = "0123456789ABCDEF";

debug_putc(string[(data >> 4) & 0x0F]);
debug_putc(string[data & 0x0F]);
}


/*
	DEBUG_PRINT_STRING()
	--------------------
*/
void debug_print_string(const char *string)
{
while (*string != 0)
	debug_putc(*string++);
}

void debug_print_bits(uint32_t i) {
	uint32_t bit;

	for (bit = 31; ; bit--) {
		if (i & (1u << bit)) {
			debug_putc('1');
		} else {
			debug_putc('0');
		}

		if (bit == 0)
			break;

		if (bit % 4 == 0)
			debug_putc(' ');
	}
}

/*
	DEBUG_PRINT_THIS()
	------------------
*/
void debug_print_this(char *start, uint32_t hex, char *end)
{
debug_print_string(start);
debug_print_hex(hex);
debug_print_string(end);
debug_print_string("\r\n");
}

void print_gpmi_state(const char *when) {
	debug_print_string("GPMI state ");
	debug_print_string(when);
	debug_print_string(":\r\n");

	debug_print_string("GPMI_DEBUG: ");
	debug_print_bits(HW_GPMI_DEBUG_RD());
	debug_print_string("\r\n");

	debug_print_string("GPMI_DEBUG2: ");
	debug_print_bits(HW_GPMI_DEBUG2_RD());
	debug_print_string("\r\n");

	debug_print_string("GPMI_DEBUG3: ");
	debug_print_bits(HW_GPMI_DEBUG3_RD());
	debug_print_string("\r\n");
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
uint32_t ATOSE_isr_irq(ATOSE_registers *registers)
{
volatile uint32_t got = 0;

/*
	Tell the ICOLL we've entered the ISR.  This is either a side-effect of the read or a write is required
*/
got = HW_ICOLL_VECTOR_RD();
got = HW_ICOLL_STAT_RD();			 // this will return VECTOR_IRQ_RTC_1MSEC

/*
	Print the ID of the interrupt we just got
*/
debug_print_string("[->");
debug_print_hex(got);
debug_print_string("<-]");

//for(;;);			// now wait

/*
	tell the GMPI that we're done
*/
HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DEV_IRQ | BM_GPMI_CTRL1_TIMEOUT_IRQ);
HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);
HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH4_ERROR_IRQ);

/*
	Tell the interrupt controller that we've finished processing the Interrupt
*/
HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);
}

/*
	GET_CPSR()
	----------
*/
uint32_t get_cpsr(void)
{
uint32_t answer;

asm volatile
	(
	"mrs %0,CPSR"
	:"=r" (answer)
	);
return answer;
}

/*
	SET_CPSR()
	----------
*/
void set_cpsr(uint32_t save_cpsr)
{
asm volatile
	(
	"msr CPSR_cxsf, %0"
	:
	:"r"(save_cpsr)
	);
}

/*
	ENABLE_IRQ()
	------------
*/
void enable_IRQ(void)
{
set_cpsr(get_cpsr() & ~0x80);
}


/*
	NS_TO_CYCLES()
	--------------
	time is is nanoseconds
	period is in cycles per second (Hz)
*/
uint32_t ns_to_cycles(unsigned int time, unsigned int period, unsigned int min)
{
uint32_t cycles;

cycles = (time + period - 1) / period;
return cycles > min ? cycles : min;
}

/*
	NS_TO_GPMI_CLOCKS()
	--------------------
*/
uint32_t ns_to_gpmi_clocks(uint32_t ns, uint32_t freq_in_mhz)
{
uint32_t clock_period_in_ns = 1000000000 / (freq_in_mhz * 1000000);

return ns_to_cycles(ns, clock_period_in_ns, 1);
}

void enable_dma_channel() {
	debug_print_string("Enable IRQ on DMA channel\r\n");
	HW_APBH_CTRL1_WR(0x00100000);		// enable IRQ on channel 4
	delay_us(1000);						// wait

	int chan = 4;

	HW_APBH_CTRL0_CLR(1 << (chan + BP_APBH_CTRL0_CLKGATE_CHANNEL)); //Ungate clock
}

void disable_dma_channel() {
	debug_print_string("Disable IRQ on DMA channel\r\n");
	HW_APBH_CTRL1_CLR(0x00100000);
	delay_us(1000);

	int chan = 4;
	HW_APBH_CTRL0_SET(1 << (chan + BP_APBH_CTRL0_CLKGATE_CHANNEL)); //Gate clock
}

void reset_dma_channel(int channel) {
	if (channel != 4) {
		debug_print_string("This doesn't work.");
	}

	debug_print_string("reset DMA channel\r\n");
	HW_APBH_CTRL0_SET(0x10 << 16);
	while (HW_APBH_CTRL0_RD() & (0xFF << 16) != 0)
		debug_putc('.');

	debug_print_string("Gate DMA channel\r\n");
	HW_APBH_CTRL0_CLR(0xFF); //Clear freeze and clock gate bits
	delay_us(1000);						// wait

	print_gpmi_state("after reset DMA channel");
}

void nand_read(char *buffer, size_t length) {
	int chip = 0;
	int                  error = 0;
	uint32_t             command_mode;
	uint32_t             address;

	struct mxs_dma_cmd * request = (dma_queue_tail++);
	struct mxs_dma_cmd * request2 = (dma_queue_tail++);

	enable_dma_channel();

	command_mode = BV_GPMI_CTRL0_COMMAND_MODE__READ;
	address      = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

	//Chain to the next request
	request->next = (uint32_t) request2;

	request->cmd.data                   = 0;
	request->cmd.bits.command           = DMA_WRITE;
	request->cmd.bits.chain             = 1;
	request->cmd.bits.irq               = 0;
	request->cmd.bits.nand_lock         = 0;
	request->cmd.bits.nand_wait_4_ready = 0;
	request->cmd.bits.dec_sem           = 1;
	request->cmd.bits.wait4end          = 1;
	request->cmd.bits.halt_on_terminate = 0;
	request->cmd.bits.terminate_flush   = 0;
	request->cmd.bits.pio_words         = 1;
	request->cmd.bits.bytes             = length;

	request->address = buffer;

	request->pio_words[0] =
		BM_GPMI_CTRL0_LOCK_CS                    |
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chip)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(length)         ;

	/*
	 * A DMA descriptor that waits for the command to end and the chip to
	 * become ready.
	 */

	command_mode = BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY;
	address      = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

	request2->cmd.data                   = 0;
	request2->cmd.bits.command           = NO_DMA_XFER;
	request2->cmd.bits.chain             = 0;
	request2->cmd.bits.irq               = 1;
	request2->cmd.bits.nand_lock         = 0;
	request2->cmd.bits.nand_wait_4_ready = 1;
	request2->cmd.bits.dec_sem           = 1;
	request2->cmd.bits.wait4end          = 1;
	request2->cmd.bits.halt_on_terminate = 0;
	request2->cmd.bits.terminate_flush   = 0;
	request2->cmd.bits.pio_words         = 4;
	request2->cmd.bits.bytes             = 0;

	request2->address = 0;

	request2->pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_LOCK_CS                    |
		BM_GPMI_CTRL0_WORD_LENGTH                |
		BF_GPMI_CTRL0_CS(chip)                   |
		BF_GPMI_CTRL0_ADDRESS(address)           |
		BF_GPMI_CTRL0_XFER_COUNT(0)              ;
	request2->pio_words[1] = 0;
	request2->pio_words[2] = 0;
	request2->pio_words[3] = 0;

	debug_print_string("NAND read of 0x");
	debug_print_hex(length);
	debug_print_string(" bytes, followed by a wait instruction.\r\n");

	HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(request));		// the address of the DMA request
	HW_APBH_CHn_SEMA_WR(4, 1);							// tell the DMA controller to issue the request
	HW_APBH_CHn_SEMA_WR(4, 1);							// can I do both of these at once...? dunno

	debug_print_string("Wait...");
	delay_us(10000);
	debug_print_string("Done\r\n");

	reset_dma_channel(4);
	disable_dma_channel();
}

void nand_send_command(char *buffer, size_t length) {
	struct mxs_dma_cmd * request = (dma_queue_tail++);
	int chip = 0;

	debug_print_this("GPMI status before: ", HW_GPMI_STAT_RD(), "\r\n");

	enable_dma_channel();

	request->next = 0;

	request->cmd.data                   = 0;
	request->cmd.bits.command           = DMA_READ;
	request->cmd.bits.chain             = 0;
	request->cmd.bits.irq               = 1;
	request->cmd.bits.nand_lock         = 0;
	request->cmd.bits.nand_wait_4_ready = 0;
	request->cmd.bits.dec_sem           = 1;
	request->cmd.bits.wait4end          = 0;
	request->cmd.bits.halt_on_terminate = 0;
	request->cmd.bits.terminate_flush   = 0;
	request->cmd.bits.pio_words         = 1;
	request->cmd.bits.bytes             = length;

	request->address = buffer;
	request->pio_words[0] =
			BM_GPMI_CTRL0_LOCK_CS                    |
			BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) |
			BM_GPMI_CTRL0_WORD_LENGTH                |
			BF_GPMI_CTRL0_CS(chip)                   |
			BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_CLE)           |
			BM_GPMI_CTRL0_ADDRESS_INCREMENT          |
			BF_GPMI_CTRL0_XFER_COUNT(length)         ;

	debug_print_string("Send NAND command, first byte:");
	debug_print_hex_byte(buffer[0]);
	debug_print_string("\r\n");

	debug_print_this("(Before incrementing semaphore:", HW_APBH_CHn_SEMA_RD(4), ")\r\n");

	HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(request));		// the address of the DMA request
	HW_APBH_CHn_SEMA_WR(4, 1);							// tell the DMA controller to issue the request

	debug_print_string("Wait...");
	delay_us(100000);
	debug_print_string("Done\r\n");

	debug_print_this("(After done semaphore:", HW_APBH_CHn_SEMA_RD(4), ")\r\n");

	reset_dma_channel(4);
	disable_dma_channel();

	debug_print_this("GPMI status after: ", HW_GPMI_STAT_RD(), "\r\n");

}

void setup_gpmi_clock() {
	/*
		Enable the GPMI clock at (initially) gpmi_freq MHz
	*/
	uint32_t gpmi_div = 480 / gpmi_freq;		// CPU speed divided by the NAND frequency

	debug_print_string("CLKCTRL_FRAC: ");
	debug_print_bits(HW_CLKCTRL_FRAC_RD());
	debug_print_string("\r\n");

	HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_IOFRAC);
	HW_CLKCTRL_FRAC_SET(BF_CLKCTRL_FRAC_IOFRAC(18));
    HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEIO); //Turn on the IO clock which the GPMI clock is derived from

    delay_us(1000);

	debug_print_string("CLKCTRL_FRAC: ");
	debug_print_bits(HW_CLKCTRL_FRAC_RD());
	debug_print_string("\r\n");

	debug_print_string("\r\n");
	debug_print_string("On boot HW_CLKCTRL_GPMI: ");
	debug_print_hex(HW_CLKCTRL_GPMI_RD());
	debug_print_string("\r\n");

	while (HW_CLKCTRL_GPMI.B.BUSY) { debug_print_string("."); }

	HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);			// start from 480MHz

	HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_CLKGATE); // CLKGATE must be cleared to set divisor

	HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);

	// Set divider
	HW_CLKCTRL_GPMI.B.DIV = gpmi_div;

	while(HW_CLKCTRL_GPMI.B.BUSY);

	debug_print_string("After setup HW_CLKCTRL_GPMI: ");
	debug_print_hex(HW_CLKCTRL_GPMI_RD());
	debug_print_string("\r\n");
}

void reset_gpmi() {
    if (!(HW_GPMI_STAT_RD() & BM_GPMI_STAT_PRESENT)) {
        debug_print_string("You're totally boned.");
    }


	/*
		reset the GPMI
	*/
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);				// make sure soft-reset is not enabled

	delay_us(1000);
	while (HW_GPMI_CTRL0.B.SFTRST) {}

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);				// clear waiting for soft-reset to set

	delay_us(1000);

	HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);				// soft reset

	debug_print_string("Wait for GPMI reset to assert CLKGATE...");

	while (!HW_GPMI_CTRL0.B.CLKGATE)	/* wait */ ;		// wait for the clock to gate

	debug_print_string(" done\r\n");

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);				// come out of reset
	delay_us(1000);										// wait a microsecond (should be 3 GPMI clocks)

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);				// enter non gated state

	debug_print_string("Wait CLKGATE to clear...");

	while (HW_GPMI_CTRL0.B.CLKGATE)	/* wait */ ;		// wait until were done.

	debug_print_string(" done.\r\n");
}

void configure_gpmi() {
	/*
		set up the GPMI for the FourARM
	*/
	while (HW_GPMI_CTRL0.B.RUN) /* wait */ ;				// wait until RUN is zero (i.e. not processing a command)
	HW_GPMI_CTRL0_SET(BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT));		// set 8-bit databus

	/*
		We must disable ENABLE before programming then re-enable afterwards.
	*/
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DLL_ENABLE);
	HW_GPMI_CTRL1_WR(BM_GPMI_CTRL1_RDN_DELAY/* | BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY*/);			// RDN delay = 0xF and polarity = RDY_BUSY is busy on low ready on high
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_GPMI_MODE); //Kernel says this switches to NAND mode
	HW_GPMI_CTRL1_SET(/* TODO BM_GPMI_CTRL1_DLL_ENABLE |*/ BM_GPMI_CTRL1_DEV_RESET);
	/*
		Now we must wait 64 GPMI cycles before continuing
	*/
	delay_us(64000);

	HW_GPMI_TIMING0_WR(
		BF_GPMI_TIMING0_ADDRESS_SETUP(ns_to_gpmi_clocks(25, gpmi_freq)) |
		BF_GPMI_TIMING0_DATA_HOLD(ns_to_gpmi_clocks(60, gpmi_freq)) |
		BF_GPMI_TIMING0_DATA_SETUP(ns_to_gpmi_clocks(80, gpmi_freq)));

	HW_GPMI_TIMING1_WR(0x00100000);	// wait for a looooong time for a timeout

	print_gpmi_state("after setup");
}

/*
	C_ENTRY()
	---------
*/
void c_entry(void)
{
uint32_t index;
uint8_t irq_stack[1024];
uint8_t *irq_sp = irq_stack + sizeof(irq_stack);

char device_id_buffer[NAND_DEVICE_ID_BYTE_COUNT];

int i;

/*
	Magic to get around the brownout problem in the FourARM
*/
HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;

/*
	Set up the IRQ stack
*/
asm volatile
	(
	"mrs r0, cpsr;"							// get the current mode
	"bic r1, r0, #0x1F;"					// turn off the mode bits
	"orr r1, r1, #0x12;"					// turn on the IRQ bits
	"msr cpsr, r1;"							// go into IRQ mode
	"mov sp, %[stack];"						// set the stack top
	"msr cpsr, r0;"							// back to original mode
	:
	: [stack]"r"(irq_sp)
	: "r0", "r1"
	);


/*
	Move the interrupt vector table to 0x00000000
*/
uint32_t p15;
asm volatile
	(
	"MRC p15, 0, R0, c1, c0, 0;"			// read control register
	"AND R0, #~(1<<13);"						// turn off the high-interrupt vector table bit
	"MCR p15, 0, R0, c1, c0, 0;"			// write control register
	:
	:
	: "r0");

/*
	Program Interrupt controller (i.MX233 ICOLL)
*/
HW_ICOLL_CTRL_WR(0);																	// reset the interrupt controller
HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_GPMI_DMA, BM_ICOLL_INTERRUPTn_ENABLE);				// Enable the interrupt in the PIC (Interrupt Level 0)
HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_GPMI, BM_ICOLL_INTERRUPTn_ENABLE);					// Enable the interrupt in the PIC (Interrupt Level 0)
HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_ARM_RSE_MODE | BM_ICOLL_CTRL_IRQ_FINAL_ENABLE);		// Tell the PIC to pass interrupts on to the CPU (and do ARM-style ISRs)

/*
	Start the GPMI
*/
nand_select_pins();

setup_gpmi_clock();

reset_gpmi();

configure_gpmi();

/*
	Enable IRQ
*/
enable_IRQ();

/*
	Now set up a DMA request to reset the chip
*/
debug_print_string("reset DMA controller\r\n");
HW_APBH_CTRL0_CLR(1 << 31);			// Soft reset
delay_us(1000);						// wait

debug_print_string("gate DMA controller\r\n");
HW_APBH_CTRL0_CLR(1 << 30);			// gate
delay_us(1000);						// wait

reset_dma_channel(4);

debug_print_string("APBH_CTRL0: ");
debug_print_bits(HW_APBH_CTRL0_RD());
debug_print_string("\r\n");
debug_print_string("APBH_CTRL1: ");
debug_print_bits(HW_APBH_CTRL1_RD());
debug_print_string("\r\n");

/*
	Now do a DMA transfer
*/
uint8_t command[2];

/*
	Reset device (command 0xFF)
*/
command[0] = 0xFF;
nand_send_command(command, 1);

command[0] = 0x70; // STATUS
nand_send_command(command, 1);

int max = 5;

char status;
do
	{
	nand_read(&status, 1);
	debug_print_this("Status register: ", status, ".\r\n");
	max--;
	delay_us(1000);
	}
while (!(status & 0x40) && max); //Bit 6 of 7

/* Get chip ID */
command[0] = 0x90;
command[1] = 0x00; //Address zero
//command[2] = 0x00; No page so we don't need to pass this
nand_send_command(command, 2);

for (i = 0; i < sizeof(device_id_buffer); i++)
	{
	device_id_buffer[i] = 0 ;
	}

nand_read(device_id_buffer, sizeof(device_id_buffer));

debug_print_string("Device ID read: ");
for (i = 0; i < sizeof(device_id_buffer); i++)
	{
	debug_print_hex_byte(device_id_buffer[i]);
	}
debug_print_string("\r\n");

//for (;;)
	{
	debug_print_this("CMD:", HW_APBH_CHn_CMD_RD(4), "");
	debug_print_this("SEM:", HW_APBH_CHn_SEMA_RD(4), "");
	debug_print_this("APBH_DEBUG1:", HW_APBH_CHn_DEBUG1_RD(4), "");
	debug_print_this("APBH_DEBUG2:", HW_APBH_CHn_DEBUG2_RD(4), "");
	debug_print_this("APBH_CTRL0:", HW_APBH_CTRL0_RD(), "");
	debug_print_this("APBH_CTRL1:", HW_APBH_CTRL1_RD(), "\r\n");
	}

for (;;);				// loop forever
}
