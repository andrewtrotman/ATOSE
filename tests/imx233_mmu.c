/*
	IMX233_MMU.C
	------------
	Experiments with the i.MX233 Memory Management Unit
*/
#include <stdint.h>
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsdigctl.h"

/*
	At the moment we'll only use 4K for the page table
*/
#define MAX_SECTIONS 0x1000
 
__attribute__ ((section (".kernel_pagetable"))) uint32_t section_table[MAX_SECTIONS];

/*
	DEBUG_PUTC()
	------------
*/
inline void debug_putc(char ch)
{
int loop = 0;

while (HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF)
	if (++loop > 10000)
		break;

HW_UARTDBGDR_WR(ch);
}

/*
	Interrupt Service Routines
*/
void __cs3_isr_undef(void) { for (;;); }
void __cs3_isr_pabort(void) { for (;;); }
void __cs3_isr_dabort(void) { for (;;); }
void __cs3_isr_reserved(void) { for (;;); }
void __cs3_isr_fiq(void) { for (;;); }
void ATOSE_isr_swi(void) { for (;;); }
void ATOSE_isr_irq(void) { for (;;); }

/*
	DELAY_US()
	----------
*/
void delay_us(unsigned long int us)
{
unsigned long int start = HW_DIGCTL_MICROSECONDS_RD();

while ((start + us) > HW_DIGCTL_MICROSECONDS_RD() )
	/* nothing */;
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

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(uint32_t data)
{
int i = 0;
char c;

for (i = sizeof(int) * 2 - 1; i >= 0; i--)
	{
	c = data >> (i * 4);
	c &= 0xf;
	if (c > 9)
		debug_putc(c - 10 + 'A');
	else
		debug_putc(c + '0');
	}
}

/*
	MMU_INIT()
	----------
*/
void mmu_init(void)
{
uint32_t section;

debug_print_string("In MMU_init\r\n");
debug_print_string("Section Table Address:");
debug_print_hex((uint32_t)(section_table));
debug_print_string("\r\n");

/*
	Set up a flat linear page table of 4096 pages of 1MB sections with "world" read/write permissions
*/
for (section = 0; section < MAX_SECTIONS; section++)
	section_table[section] = section << 20 | 0x0C12;

debug_print_string("Table initialised\r\n");

/*
	Enable the MMU
*/
asm volatile
	(
	"mrc	p15, 0, r0, c1, c0;"			// read c1
	"orr	r0, r0, #0x1000;"				// turn on the 'S' bit
	"mcr	p15, 0, r0, c1, c0, 0;"			// write c1

	"ldr	r0, =section_table;"			// address of page table
	"mcr	p15, 0, r0, c2, c0;"			// write  c2

	"mov	r0, #0xffffffff;"				// domains (16 lots of %11) telling the MMU that we're the manager
	"mcr	p15, 0, r0, c3, c0, 0;"			// write c3

	"mcr	p15, 0, r0, c7, c10, 4;"		// write c7	 drain the cache write buffer

	"mrc	p15, 0, r0, c1, c0, 0;"			// read c1
	"orr	r0, r0, #5;"					// enable data cache
	"mcr	p15, 0, r0, c1, c0, 0;"			// write c1

	"nop;"									// noop so that the pipeline fills correctly
	"nop;"
	"nop;"
	"nop;" :::"r0"
	);

debug_print_string("Done\r\n");
}


/*
	MMU_BREAK()
	-----------
*/
void mmu_break(void)
{
int section;

/*
	Break the page table
*/
for (section = 0; section < MAX_SECTIONS; section++)
	section_table[section] = 45 << 20 | 0x0C12;

/*
	Enable the MMU
*/
asm("mrc	p15, 0, r0, c1, c0");			// read c1
asm("orr	r0, r0, #0x1000");				// turn on the 'S' bit
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("ldr	r0, =section_table");			// address of page table
asm("mcr	p15, 0, r0, c2, c0");			// write  c2

asm("mov	r0, #0xffffffff");				// domains (16 lots of %11) telling the MMU that we're the manager
asm("mcr	p15, 0, r0, c3, c0, 0");		// write c3

asm("mcr	p15, 0, r0, c7, c10, 4");		// write c7	 drain the cache write buffer

asm("mrc	p15, 0, r0, c1, c0, 0");		// read c1
asm("orr	r0, r0, #5");					// enable MMU, enable data cache
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("nop");								// noop so that the pipeline fills correctly
asm("nop");
asm("nop");
asm("nop" :::"r0");
}

/*
	C_ENTRY()
	---------
*/
void c_entry(void)
{
uint32_t irq_stack[256];
uint32_t *irq_sp = irq_stack + sizeof(irq_stack);


/*
	Magic to get around the brownout problem in the FourARM
*/
HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;

/*
	Set up the IRQ stack (but we don't need this)
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
	Move the interrupt vector table to 0x00000000 (but we don't need this either)
*/
uint32_t p15;
asm volatile
	(
	"MRC p15, 0, R0, c1, c0, 0;"			// read control register
	"AND R0, #~(1<<13);"					// turn off the high-interrupt vector table bit
	"MCR p15, 0, R0, c1, c0, 0;"			// write control register
	:
	:
	: "r0"
	);


/*
	PROGRAM STARTS HERE
	-------------------
*/
debug_print_string("Enter\r\n");
mmu_init();
debug_print_string("Done MMU init\r\n");
//mmu_break();
debug_print_string("Done MMU break\r\n");

for (;;);
}
