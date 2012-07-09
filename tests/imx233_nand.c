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
current_pin_state = current_pin_state & 0xFFF03C30;
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
HW_PINCTRL_PULL0_CLR(0x001800FF);
HW_PINCTRL_PULL2_CLR(0x18000000);

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
void debug_print_string(char *string)
{
while (*string != 0)
	debug_putc(*string++);
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
	NS_TO_GPMI_CLOCKS()
	-------------------
	1ns = 1Ghz = 1000MHz = 1000000KHz = 1000000000Hz
*/
uint32_t ns_to_gpmi_clocks(uint32_t ns, uint32_t freq_in_mhz)
{
return (ns * 1000) / freq_in_mhz;
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

uint32_t gpmi_freq = 40; // MHz


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

/*
	Program the GPMI
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST | BM_GPMI_CTRL0_CLKGATE);						// enable the controller
HW_GPMI_CTRL0_SET(BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT));		// set 8-bit databus

/*
	We must disable ENABLE before programming then re-enable afterwards.
*/
HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DLL_ENABLE);
HW_GPMI_CTRL1_WR(BM_GPMI_CTRL1_RDN_DELAY | BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY);
HW_GPMI_CTRL1_SET(BM_GPMI_CTRL1_DLL_ENABLE);
/*
	Now we must wait 64 GPMI cycles before continuing
*/
delay_us(111000);


HW_GPMI_TIMING0_WR(
	BF_GPMI_TIMING0_ADDRESS_SETUP(ns_to_gpmi_clocks(25, gpmi_freq)) | 
	BF_GPMI_TIMING0_DATA_HOLD(ns_to_gpmi_clocks(60, gpmi_freq)) | 
	BF_GPMI_TIMING0_DATA_SETUP(ns_to_gpmi_clocks(80, gpmi_freq)));

//HW_GPMI_TIMING1_WR(DEVICE_BUSY_TIMEOUT);				// I guess this can be a second


/*
	Enable the GPMI clock at 40MHz
*/
uint32_t gpmi_div = 480 / gpmi_freq;		// CPU speed divided by the NAND frequency
HW_CLKCTRL_GPMI_WR((HW_CLKCTRL_GPMI_RD() & 0xFFFFFC00) | gpmi_div);
HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);			// start from 480MHz


/*
	Enable IRQ
*/
enable_IRQ();

/*
	Now set up a DMA request to reset the chip
*/
debug_print_string("reset DMA controller\n");
HW_APBH_CTRL0_CLR(1 << 31);

debug_print_string("gate DMA controller\n");
HW_APBH_CTRL0_CLR(1 << 30);

debug_print_string("reset DMA chanel 4\n");
HW_APBH_CTRL0_SET(0x10 << 16);
while (HW_APBH_CTRL0_RD() & (0xFF << 16) != 0)
	debug_putc('.');

debug_print_string("Gate DMA chanel 4\n");
HW_APBH_CTRL0_CLR(0xFF);

debug_print_string("Enable IRQ on DMA chanel 4\n");
HW_APBH_CTRL1_WR(0x00100010);		// enable IRQ on chanel 4

/*
	Now do a DMA transfer
*/
uint32_t DMA_request[20];
uint8_t DMA_answer[20];

for (index = 0; index < sizeof(DMA_answer); index++)
	DMA_answer[index] = 0;

/*
	Reset device (command 0xFF)
*/
DMA_request[0] = 0;
DMA_request[1] = 0x00001048;
DMA_request[2] = (uint32_t)DMA_answer;
DMA_request[3] = 0xFFFFFFFF;


HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(&DMA_request[0]));		// the address of the DMA request
HW_APBH_CHn_SEMA_WR(4, 1);							// tell the DMA controller to issue the request

debug_print_string("Wait...");
delay_us(11000);
debug_print_string("Done\n");

/*
	Now set up a DMA request to get the chip ID (command 90 00)
*/
DMA_request[0] = 0;
DMA_request[1] = 0x00052048 | 0x01;			// read from peripheral
DMA_request[2] = (uint32_t)DMA_answer;
DMA_request[3] = 0x90000090;


HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(&DMA_request[0]));		// the address of the DMA request
HW_APBH_CHn_SEMA_WR(4, 1);							// tell the DMA controller to issue the request

debug_print_string("Wait...");
delay_us(11000);
debug_print_string("Done\n");

//for (;;)
	{
	debug_print_this("(CMD:", HW_APBH_CHn_CMD_RD(4), ")");
	debug_print_this("(SEM:", HW_APBH_CHn_SEMA_RD(4), ")");
	debug_print_this("(DB1:", HW_APBH_CHn_DEBUG1_RD(4), ")");
	debug_print_this("(DB2:", HW_APBH_CHn_DEBUG2_RD(4), ")");
	debug_print_this("(CR0:", HW_APBH_CTRL0_RD(), ")");
	debug_print_this("(CR1:", HW_APBH_CTRL1_RD(), ")\n");
	}

for (index = 0; index < sizeof(DMA_answer); index++)
	debug_print_hex_byte(DMA_answer[index]);
debug_print_string("\n");

for (;;);				// loop forever
}
