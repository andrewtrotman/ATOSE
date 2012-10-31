/*
	IMX233_TIMER.C
	--------------
*/
#include <stdint.h>
#include "../source/registers.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsrtc.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsicoll.h"


volatile uint32_t count = 32;

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
	DISABLE_IRQ()
	-------------
*/
void disable_IRQ(void)
{
set_cpsr(get_cpsr() | 0x80);
}

/*
	ICOLL interrupt table
*/
typedef void (*void_routine_void)();
static void_routine_void interrupt_table[HW_ICOLL_INTERRUPTn_COUNT];

/*
	ATOSE_ISR_IRQ()
	---------------
*/
void ATOSE_isr_irq(ATOSE_registers *registers)
{
volatile uint32_t got = 0;

count++;

/*
	Tell the ICOLL we've entered the ISR.  This is either a side-effect of the read or a write is required
*/
got = HW_ICOLL_VECTOR_RD();
//HW_ICOLL_VECTOR_WR(0);
got = *((uint32_t *)got);

//got = HW_ICOLL_STAT_RD();			 this will return VECTOR_IRQ_RTC_1MSEC

//#ifdef NEVER
	if (got == 0x1224)
		debug_putc('Y');
	else if (got == VECTOR_IRQ_RTC_1MSEC)
		debug_putc('W');
	else
		{
		debug_putc('[');
		debug_print_hex(got);
		debug_putc(']');
		}
//#endif

/*
	tell the 1-msec timer that we're done
*/
got = HW_RTC_CTRL_RD();
HW_RTC_CTRL_CLR(BM_RTC_CTRL_ONEMSEC_IRQ);

/*
	Finished processing the Interrupt
*/
HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);
}


/*
	C_ENTRY()
	---------
*/
void c_entry(void)
{
uint32_t index;
uint8_t irq_stack[1024] __attribute__ ((aligned (4)));
uint8_t *irq_sp = irq_stack + sizeof(irq_stack);

/*
	Magic to get around the brownout problem in the FourARM
*/
HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;

/*
	Set up the IRQ stack
*/

asm volatile
	(
	"mov r2, %[stack];"
	"mrs r0, cpsr;"							// get the current mode
	"bic r1, r0, #0x1F;"					// turn off the mode bits
	"orr r1, r1, #0x12;"					// turn on the IRQ bits
	"msr cpsr, r1;"							// go into IRQ mode
	"mov sp, r2;"							// set the stack top
	"msr cpsr, r0;"							// back to original mode
	:
	: [stack]"r"(irq_sp)
	: "r0", "r1", "r2"
	);

/*
	Move the interrupt vector table to 0x00000000
*/
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

for (index = 0; index < HW_ICOLL_INTERRUPTn_COUNT; index++)
	interrupt_table[index] = 0;
interrupt_table[VECTOR_IRQ_RTC_1MSEC] = (void_routine_void)0x1224;
HW_ICOLL_VBASE_WR((uint32_t)&interrupt_table);											// ICOLL interrupt table

HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_RTC_1MSEC, BM_ICOLL_INTERRUPTn_ENABLE);				// Enable the interrupt in the PIC (Interrupt Level 0)
HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_ARM_RSE_MODE | BM_ICOLL_CTRL_IRQ_FINAL_ENABLE);		// Tell the PIC to pass interrupts on to the CPU (and do ARM-style ISRs)

/*
	Start the 1ms timer
*/
HW_RTC_CTRL_SET(BM_RTC_CTRL_ONEMSEC_IRQ_EN);

/*
	Enable IRQ
*/
enable_IRQ();

for (;;)
	{
	uint32_t got;

	got = HW_RTC_CTRL_RD();
	debug_print_hex(got);
	debug_putc(' ');
	}

for (;;);				// loop forever
}
