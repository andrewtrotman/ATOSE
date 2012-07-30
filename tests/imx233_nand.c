/*
 IMX233_NAND.C
 -------------
 Experiments with the I.MX233 GPMI
 */
#include <stdint.h>
#include <stdbool.h>
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
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsbch.h"

//Number of 512-byte blocks inside a page which follow the first block
#define NAND_NBLOCKS 7

#define NAND_PAGE_SIZE (4096 + 224)

#define NAND_PAGES_PER_BLOCK 128
#define NAND_BLOCKS_PER_LUN 2048

/**
 * struct mxs_dma_cmd_bits - MXS DMA hardware command bits.
 *
 * This structure describes the in-memory layout of the command bits in a DMA
 * command. See the appropriate reference manual for a detailed description
 * of what these bits mean to the DMA hardware.
 */
struct mxs_dma_cmd_bits {
	unsigned int command :2;
	unsigned int chain :1;
	unsigned int irq :1;
	unsigned int nand_lock :1;
	unsigned int nand_wait_4_ready :1;
	unsigned int dec_sem :1;
	unsigned int wait4end :1;
	unsigned int halt_on_terminate :1;
	unsigned int terminate_flush :1;
	unsigned int resv2 :2;
	unsigned int pio_words :4;
	unsigned int bytes :16;
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

#define NAND_ID_LEN 5

struct mxs_dma_cmd {
	void *next;
	union {
		unsigned long data;
		struct mxs_dma_cmd_bits bits;
	} cmd;
	union {
		char *address;
		unsigned long alternate;
	};
	uint32_t pio_words[DMA_PIO_WORDS];
};

struct nand_address {
	uint16_t block; //Choose a block from our complete array
	uint8_t page; //Address pages inside a block
	uint16_t column; //Address bytes inside a page
};

struct mxs_dma_cmd dma_queue[8];

__attribute__((aligned(0x4))) char pagebuf[NAND_PAGE_SIZE];
uint32_t gpmi_freq = 10; // MHz

/*
 Interrupt Service Routines
 */
void __cs3_isr_undef(void) {
}
void __cs3_isr_pabort(void) {
}
void __cs3_isr_dabort(void) {
}
void __cs3_isr_reserved(void) {
}
void __cs3_isr_fiq(void) {
}
void ATOSE_isr_swi(void) {
}


uint8_t twiddle(uint8_t b) {
	typedef union {
		struct {
			unsigned char bit0 :1;
			unsigned char bit1 :1;
			unsigned char bit2 :1;
			unsigned char bit3 :1;
			unsigned char bit4 :1;
			unsigned char bit5 :1;
			unsigned char bit6 :1;
			unsigned char bit7 :1;
		} u;
		unsigned char b;
	} input_order;

	typedef union {
		struct {
			unsigned char bit0 :1;
			unsigned char bit1 :1;
			unsigned char bit2 :1;
			unsigned char bit3 :1;
			unsigned char bit5 :1;
			unsigned char bit4 :1;
			unsigned char bit7 :1;
			unsigned char bit6 :1;
		} u;
		unsigned char b;
	} output_order;

	input_order input;
	output_order output;

	input.b = b;
	output.u.bit0 = input.u.bit0;
	output.u.bit1 = input.u.bit1;
	output.u.bit2 = input.u.bit2;
	output.u.bit3 = input.u.bit3;
	output.u.bit4 = input.u.bit4;
	output.u.bit5 = input.u.bit5;
	output.u.bit6 = input.u.bit6;
	output.u.bit7 = input.u.bit7;

	return output.b;
}

struct nand_address nand_make_address(uint16_t block, uint8_t page, uint16_t column) {
	struct nand_address result;

	result.block = block;
	result.page = page;
	result.column = column;

	return result;
}

/**
 * Convert a linear page index to a page address in the NAND's 524,288
 * total pages.
 */
struct nand_address nand_page_index_to_address(uint32_t page_index) {
	return	nand_make_address(
			page_index / NAND_PAGES_PER_BLOCK,
			page_index % NAND_PAGES_PER_BLOCK,
			0
	);
}

/*
 DEBUG_PUTC()
 ------------
 */
void debug_putc(char ch) {
	int loop = 0;

	while (HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF)
		if (++loop > 10000)
			break;

	/* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
	HW_UARTDBGDR_WR(ch);
}

/*
 DEBUG_PRINT_HEX()
 -----------------
 */
void debug_print_hex(int data) {
	int i = 0;
	char c;

	for (i = sizeof(int) * 2 - 1; i >= 0; i--) {
		c = data >> (i * 4);
		c &= 0xf;
		if (c > 9)
			debug_putc(c - 10 + 'A');
		else
			debug_putc(c + '0');
	}
}

/*
 DEBUG_PRINT_HEX()
 -----------------
 */
void debug_print_hex_byte(uint8_t data) {
	char *string = "0123456789ABCDEF";

	debug_putc(string[(data >> 4) & 0x0F]);
	debug_putc(string[data & 0x0F]);
}

/*
 DEBUG_PRINT_STRING()
 --------------------
 */
void debug_print_string(const char *string) {
	while (*string != 0)
		debug_putc(*string++);
}

void debug_print_bits(uint32_t i) {
	uint32_t bit;

	for (bit = 31;; bit--) {
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
void debug_print_this(char *start, uint32_t hex, char *end) {
	debug_print_string(start);
	debug_print_hex(hex);
	debug_print_string(end);
	debug_print_string("\r\n");
}

void debug_print_this_bits(char *start, uint32_t hex, char *end) {
	debug_print_string(start);
	debug_print_bits(hex);
	debug_print_string(end);
	debug_print_string("\r\n");
}

void debug_print_buffer(char * buffer, int length, int abs_addr) {

	int i, j;
	int colsize = 32;

	debug_print_string("\r\n");

	for (i = 0; i < length; i++) {
		//Print leading address at beginning of line
		if (i % colsize == 0) {
			if (abs_addr) {
				debug_print_hex((uint32_t) buffer + i);
			} else {
				debug_print_hex(i);
			}
			debug_putc(' ');
		}

		debug_print_hex_byte(buffer[i]);

		/*
		 * Print blanks up til the end of the line if we're finishing up.
		 */
		if (i == length - 1 && length % colsize != 0) {
			for (j = 0; j < colsize - length % colsize; j++) {
				debug_print_string("  ");
			}
		}

		//Print ASCII representation at end of line
		if (i % colsize == colsize - 1 || i == length - 1) {
			debug_print_string("  ");

			for (j = 0; j < colsize; j++) {
				char c = buffer[i + j - colsize + 1];

				if (c >= 32 && c < 127) {
					debug_putc(c);
				} else {
					debug_putc('?');
				}
			}

			debug_print_string("\r\n");
		}
	}
}

void print_gpmi_state(const char *when) {
	debug_print_string("GPMI state ");
	debug_print_string(when);
	debug_print_string(":\r\n");

	debug_print_string("GPMI_DEBUG: ");
	debug_print_bits(HW_GPMI_DEBUG_RD() );
	debug_print_string("\r\n");

	debug_print_string("GPMI_DEBUG2: ");
	debug_print_bits(HW_GPMI_DEBUG2_RD() );
	debug_print_string("\r\n");

	debug_print_string("GPMI_DEBUG3: ");
	debug_print_bits(HW_GPMI_DEBUG3_RD() );
	debug_print_string("\r\n");
}

/*
 NAND_SELECT_PINS()
 ------------------
 */
void nand_select_pins(void) {
	uint32_t current_pin_state;

	/*
	 Tell the i.MX233 to use the pins for GPMI
	 */

	//Turn on the 8-bit data bus and leave the other pins how they were
	current_pin_state = HW_PINCTRL_MUXSEL0_RD();
	current_pin_state = current_pin_state & 0xFFFF0000;
	HW_PINCTRL_MUXSEL0_WR(current_pin_state);

	//Turn on the control lines
	current_pin_state = HW_PINCTRL_MUXSEL1_RD();
	current_pin_state = current_pin_state & 0xFFF00000; //& 0xFFF03C30;
	HW_PINCTRL_MUXSEL1_WR(current_pin_state);

	current_pin_state = HW_PINCTRL_MUXSEL5_RD();
	current_pin_state = current_pin_state & 0xFc3FFFFF;
	HW_PINCTRL_MUXSEL5_WR(current_pin_state);

	/*
	 Tell the i.MX233 to use the right voltages and current
	 */

	//Data bus at 4mA
	HW_PINCTRL_DRIVE0_WR(0x00000000);

	//drive WPN at 12mA and the other control lines at 4mA
	HW_PINCTRL_DRIVE2_WR(0x10000000);

	//RDN and WRN at 12mA
	HW_PINCTRL_DRIVE3_WR((HW_PINCTRL_DRIVE3_RD() & 0xFFFFFF00) | 0x22);

	//and the remaining control lines at 4mA
	HW_PINCTRL_DRIVE11_WR(HW_PINCTRL_DRIVE11_RD() & 0xFFF00FFF);

	//Now turn off the pull-up resistors (turn on the pad keepers)
	HW_PINCTRL_PULL0_CLR(0x001800FF);
	HW_PINCTRL_PULL2_CLR(0x18000000);

	HW_PINCTRL_PULL0_SET(
		BM_PINCTRL_PULL0_BANK0_PIN19
		| BM_PINCTRL_PULL0_BANK0_PIN20
	);

	/*
	 We're done - yay!
	 */
}

/*
 DELAY_US()
 ----------
 */
void delay_us(unsigned long int us) {
	unsigned long int start = HW_DIGCTL_MICROSECONDS_RD();

	while ((start + us) > HW_DIGCTL_MICROSECONDS_RD() )
	/* nothing */
		;
}

/*
 ATOSE_ISR_IRQ()
 ---------------
 */
uint32_t ATOSE_isr_irq(ATOSE_registers *registers) {
	volatile uint32_t got = 0;

	/*
	 Tell the ICOLL we've entered the ISR.  This is either a side-effect of the read or a write is required
	 */
	got = HW_ICOLL_VECTOR_RD();
	got = HW_ICOLL_STAT_RD();			// this will return VECTOR_IRQ_RTC_1MSEC

	/*
	 Print the ID of the interrupt we just got
	 */

	 debug_print_string("[->");
	 debug_print_hex(got);
	 debug_print_string("<-]");


	/*
	 tell the GMPI that we're done
	 */
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DEV_IRQ | BM_GPMI_CTRL1_TIMEOUT_IRQ);
	//Also the APBH:
	HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);
	HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH4_ERROR_IRQ);
	HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH5_CMDCMPLT_IRQ);
	HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH5_ERROR_IRQ);
	//Also the BCH: TODO
	HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ);

	/*
	 Tell the interrupt controller that we've finished processing the Interrupt
	 */
	HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);
}

/*
 GET_CPSR()
 ----------
 */
uint32_t get_cpsr(void) {
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
void set_cpsr(uint32_t save_cpsr) {
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
void enable_IRQ(void) {
	set_cpsr(get_cpsr() & ~0x80);
}

/*
 NS_TO_CYCLES()
 --------------
 time is is nanoseconds
 period is in cycles per second (Hz)
 */
uint32_t ns_to_cycles(unsigned int time, unsigned int period, unsigned int min) {
	uint32_t cycles;

	cycles = (time + period - 1) / period;
	return cycles > min ? cycles : min;
}

/*
 NS_TO_GPMI_CLOCKS()
 --------------------
 */
uint32_t ns_to_gpmi_clocks(uint32_t ns, uint32_t freq_in_mhz) {
	uint32_t clock_period_in_ns = 1000000000 / (freq_in_mhz * 1000000);

	return ns_to_cycles(ns, clock_period_in_ns, 1);
}

void enable_dma_channel(int channel) {
	uint32_t channel_id;

	if (channel == 4) {
		HW_APBH_CTRL1.B.CH4_CMDCMPLT_IRQ_EN = 1;
	} else if (channel == 5) {
		HW_APBH_CTRL1.B.CH5_CMDCMPLT_IRQ_EN = 1;
	} else {
		debug_print_string("This doesn't work.");
	}

	delay_us(1000);

	HW_APBH_CTRL0_CLR(1 << (channel + BP_APBH_CTRL0_CLKGATE_CHANNEL)); //Ungate clock
}

/*void disable_dma_channel() {
	debug_print_string("This doesn't work\r\n");

	debug_print_string("Disable IRQ on DMA channel\r\n");
	HW_APBH_CTRL1_CLR(0x00100000);
	delay_us(1000);

	int chan = 4;
	HW_APBH_CTRL0_SET(1 << (chan + BP_APBH_CTRL0_CLKGATE_CHANNEL)); //Gate clock
}*/

void reset_dma_channel(int channel) {
	uint32_t channel_id;

	if (channel == 4) {
		channel_id = BV_APBH_CTRL0_RESET_CHANNEL__NAND0;
	} else if (channel == 5) {
		channel_id = BV_APBH_CTRL0_RESET_CHANNEL__NAND1;
	} else {
		debug_print_string("This doesn't work.");
	}

	HW_APBH_CTRL0_SET(BF_APBH_CTRL0_RESET_CHANNEL(channel_id));

	while (HW_APBH_CTRL0_RD() & BF_APBH_CTRL0_RESET_CHANNEL(channel_id) != 0)
		debug_putc('.');

	HW_APBH_CTRL0_CLR(0xFF); //Clear freeze bits
	delay_us(1000);						// wait
}

/* Provide a buffer of length 4096 bytes to read a complete ECC page */
void nand_write_ecc(int chip, char *buffer, size_t length) {
	int error = 0;
	int i;

	struct mxs_dma_cmd * request = &dma_queue[0];
	struct mxs_dma_cmd * request2 = &dma_queue[1];
	char command[1];

	request->next = request2;

	request->cmd.data					= 0;
	request->cmd.bits.command			= BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
	request->cmd.bits.chain				= 1;
	request->cmd.bits.irq				= 0;
	request->cmd.bits.nand_lock			= 0;
	request->cmd.bits.nand_wait_4_ready	= 0;
	request->cmd.bits.dec_sem			= 0;
	request->cmd.bits.wait4end			= 1;
	request->cmd.bits.halt_on_terminate	= 0;
	request->cmd.bits.terminate_flush	= 0;
	request->cmd.bits.pio_words			= 6;
	request->cmd.bits.bytes				= 0;

	request->address = 0;

	request->pio_words[0] =
		BM_GPMI_CTRL0_LOCK_CS |
		BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) |
		BF_GPMI_CTRL0_XFER_COUNT(0);

	request->pio_words[1] = 0;

	request->pio_words[2] =
		BF_GPMI_ECCCTRL_HANDLE(0x321) |
		BF_GPMI_ECCCTRL_ECC_CMD(BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT) |
		BF_GPMI_ECCCTRL_ENABLE_ECC(1) |
		BF_GPMI_ECCCTRL_BUFFER_MASK(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE);
	request->pio_words[3] = NAND_PAGE_SIZE;

	if (((uint32_t)buffer & 0x3) != 0) {
		debug_print_string("ECC read buffer is not 32-bit aligned!");
	}

	request->pio_words[4] = (uint32_t) buffer;

	request->pio_words[5] = (uint32_t) 0;

	/*
	 * Wait for the write to finish, then send a 0x10 to signal end of data and disable
	 * ECC.
	 */
	request2->cmd.data = 0;
	request2->cmd.bits.command = BV_APBH_CHn_CMD_COMMAND__DMA_READ;
	request2->cmd.bits.chain = 0;
	request2->cmd.bits.irq = 1;
	request2->cmd.bits.nand_lock = 0;
	request2->cmd.bits.nand_wait_4_ready = 0;
	request2->cmd.bits.dec_sem = 1;
	request2->cmd.bits.wait4end = 1;
	request2->cmd.bits.halt_on_terminate = 0;
	request2->cmd.bits.terminate_flush = 0;
	request2->cmd.bits.pio_words = 3;
	request2->cmd.bits.bytes = 1;

	command[0] = twiddle(0x10);
	request2->address = command;

	request2->pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) |
		BM_GPMI_CTRL0_LOCK_CS |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_CLE) |
		BF_GPMI_CTRL0_XFER_COUNT(1);

	request2->pio_words[1] = 0;

	request2->pio_words[2] = 0; //Reset ECC

	HW_APBH_CHn_NXTCMDAR_WR(chip == 0 ? 4 : 5, (uint32_t)(request)); // the address of the DMA request
	HW_APBH_CHn_SEMA_WR(chip == 0 ? 4 : 5, 1); // tell the DMA controller to issue the request

	delay_us(100000);
}

/*
 * After issuing a READ to the device, call this routine to read a page of data
 * from the NAND using ECC. This routine finishes by ending the READ command.
 *
 * TODO: This command shouldn't be responsible for finishing the READ command but
 * this requires that the caller can queue up a DMA command after ours which they
 * currently can't.
 *
 * Provide a buffer of length 4096+224 bytes to read a complete ECC page
 */
void nand_read_ecc_page(int chip, char *buffer, size_t length, char *aux) {
	int error = 0;
	int i;

	struct mxs_dma_cmd * request = &dma_queue[0];
	struct mxs_dma_cmd * request2 = &dma_queue[1];

	request->next = request2;

	request->cmd.data 				= 0;
	request->cmd.bits.command 		= BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
	request->cmd.bits.chain 		= 1;
	request->cmd.bits.irq 			= 0;
	request->cmd.bits.nand_lock 	= 0;
	request->cmd.bits.nand_wait_4_ready = 0;
	request->cmd.bits.dec_sem 		= 1;
	request->cmd.bits.wait4end 		= 1;
	request->cmd.bits.halt_on_terminate = 0;
	request->cmd.bits.terminate_flush = 0;
	request->cmd.bits.pio_words 	= 6;
	request->cmd.bits.bytes 		= 0;

	request->pio_words[0] =
		BM_GPMI_CTRL0_LOCK_CS|
		BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__READ) |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) |
		BF_GPMI_CTRL0_XFER_COUNT(length);

	request->pio_words[1] = 0;

	request->pio_words[2] =
		BF_GPMI_ECCCTRL_HANDLE(0x123) |
		BF_GPMI_ECCCTRL_ECC_CMD(BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT) |
		BF_GPMI_ECCCTRL_ENABLE_ECC(1) |
		BF_GPMI_ECCCTRL_BUFFER_MASK(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE);

	request->pio_words[3] = length;

	if (((uint32_t)buffer & 0x3) != 0) {
		debug_print_string("ECC read buffer is not 32-bit aligned!");
	}

	if (((uint32_t)aux & 0x3) != 0) {
		debug_print_string("ECC aux buffer is not 32-bit aligned!");
	}

	request->pio_words[4] = (uint32_t) buffer;

	request->pio_words[5] = (uint32_t) aux;

	/*
	 * Waits for the command to end and disable ECC
	 */
	request2->cmd.data = 0;
	request2->cmd.bits.command = BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
	request2->cmd.bits.chain = 0;
	request2->cmd.bits.irq = 1;
	request2->cmd.bits.nand_lock = 0;
	request2->cmd.bits.nand_wait_4_ready = 1;
	request2->cmd.bits.dec_sem = 1;
	request2->cmd.bits.wait4end = 1;
	request2->cmd.bits.halt_on_terminate = 0;
	request2->cmd.bits.terminate_flush = 0;
	request2->cmd.bits.pio_words = 3;
	request2->cmd.bits.bytes = 0;

	request2->address = 0;

	request2->pio_words[0] =
		BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY) |
		BM_GPMI_CTRL0_LOCK_CS |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) |
		BF_GPMI_CTRL0_XFER_COUNT(0);

	request2->pio_words[1] = 0;

	request2->pio_words[2] = 0; //Reset ECC

	HW_APBH_CHn_NXTCMDAR_WR(chip == 0 ? 4 : 5, (uint32_t)(request)); // the address of the DMA request
	HW_APBH_CHn_SEMA_WR(chip == 0 ? 4 : 5, 1); // tell the DMA controller to issue the request
	HW_APBH_CHn_SEMA_WR(chip == 0 ? 4 : 5, 1); // tell the DMA controller to issue the request

	delay_us(100000);
}

/**
 * Read data that is waiting on the NAND bus.
 */
void nand_read(int chip, char *buffer, size_t length) {
	int error = 0;
	uint32_t command_mode;
	uint32_t address;
	int i;

	struct mxs_dma_cmd * request = &dma_queue[0]; //No queuing

	command_mode = BV_GPMI_CTRL0_COMMAND_MODE__READ;
	address = BV_GPMI_CTRL0_ADDRESS__NAND_DATA;

	//Chain to the next request
	request->next = 0;

	request->cmd.data = 0;
	request->cmd.bits.command = BV_APBH_CHn_CMD_COMMAND__DMA_WRITE;
	request->cmd.bits.chain = 0;
	request->cmd.bits.irq = 1;
	request->cmd.bits.nand_lock = 0;
	request->cmd.bits.nand_wait_4_ready = 0;
	request->cmd.bits.dec_sem = 1;
	request->cmd.bits.wait4end = 1;
	request->cmd.bits.halt_on_terminate = 0;
	request->cmd.bits.terminate_flush = 0;
	request->cmd.bits.pio_words = 3;
	request->cmd.bits.bytes = length;

	request->address = buffer;

	request->pio_words[0] =
		BM_GPMI_CTRL0_LOCK_CS |
		BF_GPMI_CTRL0_COMMAND_MODE(command_mode) |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(address) |
		BF_GPMI_CTRL0_XFER_COUNT(length);

	request->pio_words[1] = 0;

	request->pio_words[2] = 0;

	HW_APBH_CHn_NXTCMDAR_WR(chip == 0 ? 4 : 5, (uint32_t)(request)); // the address of the DMA request
	HW_APBH_CHn_SEMA_WR(chip == 0 ? 4 : 5, 1); // tell the DMA controller to issue the request

	delay_us(100000);

	for (i = 0; i < length; i++) {
		buffer[i] = twiddle(buffer[i]);
	}
}

/**
 * Issue a command (and possibly additional address bytes) to the NAND
 */
void nand_send_command(int chip, char *buffer, size_t length) {
	struct mxs_dma_cmd * request = &dma_queue[0];
	int i;

	for (i = 0; i < length; i++) {
		buffer[i] = twiddle(buffer[i]);
	}

	request->next = 0;

	request->cmd.data = 0;
	request->cmd.bits.command = BV_APBH_CHn_CMD_COMMAND__DMA_READ;
	request->cmd.bits.chain = 1;
	request->cmd.bits.irq = 1;
	request->cmd.bits.nand_lock = 0;
	request->cmd.bits.nand_wait_4_ready = 0;
	request->cmd.bits.dec_sem = 1;
	request->cmd.bits.wait4end = 1;
	request->cmd.bits.halt_on_terminate = 0;
	request->cmd.bits.terminate_flush = 0;
	request->cmd.bits.pio_words = 3;
	request->cmd.bits.bytes = length;

	request->address = buffer;

	request->pio_words[0] =
		BM_GPMI_CTRL0_LOCK_CS |
		BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) |
		BM_GPMI_CTRL0_WORD_LENGTH |
		BF_GPMI_CTRL0_CS(chip) |
		BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_CLE) |
		BM_GPMI_CTRL0_ADDRESS_INCREMENT |
		BF_GPMI_CTRL0_XFER_COUNT(length);

	request->pio_words[1] = 0;

	request->pio_words[2] = 0;

	int channel = chip == 0 ? 4 : 5;

	HW_APBH_CHn_NXTCMDAR_WR(channel, (uint32_t)(request)); // the address of the DMA request
	HW_APBH_CHn_SEMA_WR(channel, 1); // tell the DMA controller to issue the request

	delay_us(100000);
}

void setup_gpmi_clock() {
	/*
	 Enable the GPMI clock at (initially) gpmi_freq MHz
	 */
	uint32_t gpmi_div = 480 / gpmi_freq;// CPU speed divided by the NAND frequency

	HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_IOFRAC);
	HW_CLKCTRL_FRAC_SET(BF_CLKCTRL_FRAC_IOFRAC(18));
	//Set divisor for the IO clock to 18/18, i.e. run at same rate as CPU
	HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEIO);
	//Turn on the IO clock which the GPMI clock is derived from

	delay_us(1000);

	while (HW_CLKCTRL_GPMI.B.BUSY) {
	}

	HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI); // start from 480MHz
	HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_CLKGATE); // CLKGATE must be cleared to set divisor
	HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);

	HW_CLKCTRL_GPMI.B.DIV = gpmi_div; // Set GPMI clock divider from io_clk

	while (HW_CLKCTRL_GPMI.B.BUSY)
		;
}

void reset_gpmi() {
	if (!(HW_GPMI_STAT_RD() & BM_GPMI_STAT_PRESENT)) {
		debug_print_string("You're totally boned.");
	}

	// reset the GPMI
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST); // make sure soft-reset is not enabled

	delay_us(1000);
	while (HW_GPMI_CTRL0.B.SFTRST)
		; //Wait for soft reset to clear

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE); // clear waiting for soft-reset to set

	delay_us(1000);

	HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST); // soft reset

	while (!HW_GPMI_CTRL0.B.CLKGATE)
		/* wait */;		// wait for the clock to gate

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST); // come out of reset

	delay_us(1000);				// wait a microsecond (should be 3 GPMI clocks)

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE); // enter non gated state

	while (HW_GPMI_CTRL0.B.CLKGATE)
		/* wait */;		// wait until were done.

	//Done.
}

void configure_gpmi() {
	/*
	 set up the GPMI for the FourARM
	 */
	while (HW_GPMI_CTRL0.B.RUN)
		/* wait */;	// wait until RUN is zero (i.e. not processing a command)

	HW_GPMI_CTRL0_SET(
		BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) 	// set 8-bit databus
	);

	//We must disable ENABLE before programming then re-enable afterwards.
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DLL_ENABLE);
	HW_GPMI_CTRL1_WR(BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY);

	// RDN delay = 0xF and polarity = RDY_BUSY is busy on low ready on high
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_GPMI_MODE);

	//Switch to NAND mode and select BCH as our ECC
	HW_GPMI_CTRL1_SET(
			BM_GPMI_CTRL1_DEV_RESET |
			BM_GPMI_CTRL1_BCH_MODE
	);

	//Now we must wait 64 GPMI cycles before continuing
	delay_us(64000);

	HW_GPMI_TIMING0_WR(
		BF_GPMI_TIMING0_ADDRESS_SETUP(ns_to_gpmi_clocks(25, gpmi_freq))
		| BF_GPMI_TIMING0_DATA_HOLD(ns_to_gpmi_clocks(60, gpmi_freq))
		| BF_GPMI_TIMING0_DATA_SETUP(ns_to_gpmi_clocks(80, gpmi_freq))
	);

	HW_GPMI_TIMING1_WR(0x00100000); // wait for a looooong time for a timeout
}

// Pass a NAND_ID_LEN size buffer to receive the NAND ID string
// Note only chip=0 seems to work
void nand_identify(int chip, char *buffer) {
	char command[2];

	command[0] = 0x90;
	command[1] = 0x00;

	nand_send_command(chip, command, sizeof(command));
	nand_read(chip, buffer, NAND_ID_LEN);
}

//Pass a NAND_UNIQUE_ID_LEN size buffer to receive the unique id of the given chip
bool nand_identify_unique(int chip, char *buffer) {
	char command[2];
	char id[32];
	int i, retry;
	bool broken;

	command[0] = 0xED; // READ UNIQUE ID
	command[1] = 0x00;

	nand_send_command(chip, command, sizeof(command));

	broken = true;
	for (retry = 0; retry < 16 && broken; retry++) {
		nand_read(chip, id, 32);

		//Verify that the first 16 bytes XORed with the second all give 0xFF
		broken = false;
		for (i = 0; i < 16; i++) {
			if ((id[i] ^ id[i+16]) != 0xFF) {
				broken = true;
				break;
			}
		}
	}

	for (i = 0; i < 16; i++) {
		buffer[i] = id[i];
	}

	return !broken;
}

//Caller provides 256 bytes buffer
void nand_read_parameter_page(int chip, char *buffer) {
	char command[2];
	char id[32];
	int i, retry;
	bool broken;

	command[0] = 0xEC; // READ PARAMETER PAGE
	command[1] = 0x00;

	nand_send_command(chip, command, sizeof(command));

	nand_read(chip, buffer, 256);
}

uint8_t nand_status(int chip) {
	uint8_t status;
	uint8_t command;

	command = 0x70; // STATUS
	nand_send_command(chip, &command, 1);

	nand_read(chip, &status, 1);

	return status;
}

void nand_reset(int chip) {
	uint8_t command, status;

	command = 0xFF; // RESET
	nand_send_command(chip, &command, 1);

	//Poll and wait for the NAND to become ready
	do {
		status = nand_status(chip);
	} while ((status & 0x60) != 0x60); //Two ready bits should be zero

	debug_print_string("NAND status: ");
	debug_print_hex_byte(status);
	debug_print_string(".\r\n");
}

void setup_bch() {
	HW_BCH_CTRL_CLR(BM_BCH_CTRL_SFTRST); // come out of reset

	delay_us(1000);				// wait a microsecond (should be 3 BCH clocks)

	HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE); // enter non gated state

	while (HW_BCH_CTRL.B.CLKGATE)
		/* wait */;		// wait until were done.

	//Done.
	HW_BCH_FLASH0LAYOUT0_WR(
		BF_BCH_FLASH0LAYOUT0_NBLOCKS(NAND_NBLOCKS) |
		BF_BCH_FLASH0LAYOUT0_META_SIZE(0) |
		BF_BCH_FLASH0LAYOUT0_ECC0(BV_BCH_FLASH0LAYOUT0_ECC0__ECC16) |
		BF_BCH_FLASH0LAYOUT0_DATA0_SIZE(512)
	);

	HW_BCH_FLASH0LAYOUT1_WR(
		BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(NAND_PAGE_SIZE) |
		BF_BCH_FLASH0LAYOUT1_ECCN(BV_BCH_FLASH0LAYOUT1_ECCN__ECC16) |
		BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(512)
	);

	//Clear clkgate, and we only want one other flag turned on:
	HW_BCH_CTRL_WR(
		BM_BCH_CTRL_COMPLETE_IRQ_EN
		| BM_BCH_CTRL_DEBUG_STALL_IRQ_EN
	);
}

void nand_read_page(struct nand_address address, char *pagebuf) {
	__attribute__((aligned(0x4))) char metabuf[NAND_NBLOCKS + 1];
	char command[6];
	int i;

	command[0] = 0; //READ PAGE

	command[1] = address.column & 0xFF;
	command[2] = (address.column >> 8) & 0x1F;
	command[3] = (address.page & 0x7F) | ((address.block & 0x01) << 7);
	command[4] = (address.block >> 1) & 0xFF;
	command[5] = (address.block >> 9) & 0x03;

	nand_send_command(0, command, 6);

	command[0] = 0x30;

	nand_send_command(0, command, 1);

	nand_read_ecc_page(0, pagebuf, NAND_PAGE_SIZE, metabuf);

/*
	debug_print_string("\r\nMetadata: ");

	for (i = 0; i < sizeof(metabuf); i++) {
		debug_print_hex_byte(metabuf[i]);
	}

	debug_print_string("\r\n");
*/

	/*
	 * TODO examine metabuf to see if the read succeeded and return a useful status
	 * to the caller.
	 */
}

void nand_write_page(struct nand_address address, const char * buffer) {
	char command[6];

	command[0] = 0x80; // PROGRAM

	command[1] = address.column & 0xFF;
	command[2] = (address.column >> 8) & 0x1F;
	command[3] = (address.page & 0x7F) | ((address.block & 0x01) << 7);
	command[4] = (address.block >> 1) & 0xFF;
	command[5] = (address.block >> 9) & 0x03;

	nand_send_command(0, command, 6);

	nand_write_ecc(0, pagebuf, 4096);

	debug_print_string("NAND status after ECC write: ");
	debug_print_hex_byte(nand_status(0));
	debug_print_string(".\r\n");
}

void nand_erase_block(struct nand_address address) {
	char command[4];

	if (address.page || address.column) {
		debug_print_string("Block erase: Ignoring page/column address\r\n");
	}

	command[0] = 0x60; //ERASE BLOCK

	command[1] = 0 | ((address.block & 0x01) << 7);
	command[2] = (address.block >> 1) & 0xFF;
	command[3] = (address.block >> 9) & 0x03;

	nand_send_command(0, command, 4);

	command[0] = 0xD0; //Finish

	nand_send_command(0, command, 1);

	//TODO poll for completion / wait on interrupt
}

/*
 C_ENTRY()
 ---------
 */
void c_entry(void) {
	uint32_t index;
	uint32_t irq_stack[256];
	uint32_t *irq_sp = irq_stack + sizeof(irq_stack);

	char buffer[256];
	int i;

	//Magic to get around the brownout problem in the FourARM
	HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;

	//Set up the IRQ stack
	asm volatile
	(
			"mov r2, %[stack];"
			"mrs r0, cpsr;"							// get the current mode
			"bic r1, r0, #0x1F;"// turn off the mode bits
			"orr r1, r1, #0x12;"// turn on the IRQ bits
			"msr cpsr, r1;"// go into IRQ mode
			"mov sp, r2;"// set the stack top
			"msr cpsr, r0;"// back to original mode
			:
			: [stack]"r"(irq_sp)
			: "r0", "r1", "r2"
	);

	/*
	 Move the interrupt vector table to 0x00000000
	 */
	uint32_t p15;
	asm volatile
	(
			"MRC p15, 0, R0, c1, c0, 0;"			// read control register
			"AND R0, #~(1<<13);"// turn off the high-interrupt vector table bit
			"MCR p15, 0, R0, c1, c0, 0;"// write control register
			:
			:
			: "r0");

	/*
	 Program Interrupt controller (i.MX233 ICOLL)
	 */
	HW_ICOLL_CTRL_WR(0);
	// reset the interrupt controller
	HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_GPMI_DMA, BM_ICOLL_INTERRUPTn_ENABLE);
	// Enable the interrupt in the PIC (Interrupt Level 0)
	HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_GPMI, BM_ICOLL_INTERRUPTn_ENABLE);
	// Enable BCH
	HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_BCH, BM_ICOLL_INTERRUPTn_ENABLE);
	// Enable the interrupt in the PIC (Interrupt Level 0)
	HW_ICOLL_CTRL_SET(
			BM_ICOLL_CTRL_ARM_RSE_MODE |
			BM_ICOLL_CTRL_IRQ_FINAL_ENABLE
	);
	// Tell the PIC to pass interrupts on to the CPU (and do ARM-style ISRs)

	/*
	 Start the GPMI
	 */
	nand_select_pins();

	setup_gpmi_clock();

	reset_gpmi();

	configure_gpmi();

	setup_bch();

	/*
	 Enable IRQ
	 */
	enable_IRQ();

	/*
	 Now set up a DMA request to reset the chip
	 */
	debug_print_string("reset DMA controller\r\n");
	HW_APBH_CTRL0_CLR(1 << 31); // Soft reset
	delay_us(1000);						// wait

	debug_print_string("gate DMA controller\r\n");
	HW_APBH_CTRL0_CLR(1 << 30); // gate
	delay_us(1000);						// wait

	reset_dma_channel(4);
	reset_dma_channel(5);

	enable_dma_channel(4);
	enable_dma_channel(5);

	nand_reset(0);
	nand_reset(1);

	nand_identify(0, buffer);

	debug_print_string("Device ID read: ");
	for (i = 0; i < NAND_ID_LEN; i++) {
		debug_print_hex_byte(buffer[i]);
	}
	debug_print_string("\r\n");

/*
	if (nand_identify_unique(0, buffer)) {
		debug_print_string("Device unique ID: ");
		for (i = 0; i < 16; i++) {
			debug_print_hex_byte(buffer[i]);
		}
		debug_print_string("\r\n");

	} else {
		debug_print_string("Identify unique failed\r\n");
	}
*/

//  int column = 0x0000C5D; potentially broken sector on fourarm 3

	struct nand_address test_page = nand_page_index_to_address(123);

	nand_erase_block(test_page);

	//Print what was already in the page...
	nand_read_page(test_page, pagebuf);

	debug_print_buffer(pagebuf, 4096, 0);

	//Write incrementing bytes into the page as a test...
	for (i = 0; i < sizeof(pagebuf); i++) {
		pagebuf[i] = i & 0xFF;
	}

	nand_write_page(test_page, pagebuf);

	//Clear out the page buf so we can test that read() can read back the data
	for (i = 0; i < sizeof(pagebuf); i++) {
		pagebuf[i] = 0;
	}

	nand_read_page(test_page, pagebuf);

	debug_print_buffer(pagebuf, 4096, 0);

//for (;;)
	{
		debug_print_this_bits("CMD:", HW_APBH_CHn_CMD_RD(4), "");
		debug_print_this_bits("SEM:", HW_APBH_CHn_SEMA_RD(4), "");
		debug_print_this_bits("APBH_DEBUG1:", HW_APBH_CHn_DEBUG1_RD(4), "");
		debug_print_this_bits("APBH_DEBUG2:", HW_APBH_CHn_DEBUG2_RD(4), "");
		debug_print_this_bits("APBH_CTRL0:", HW_APBH_CTRL0_RD(), "");
		debug_print_this_bits("APBH_CTRL1:", HW_APBH_CTRL1_RD(), "\r\n");
	}

	for (;;)
		;				// loop forever
}
