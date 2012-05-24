/*
	TIMERII.C
	---------
	Verify the system timer interrupt mechanism (this is written for the ARM versatile ab board)
	this version uses custom I/O and custom startup (no CRTL)
*/
typedef unsigned int uint32_t;
#define UART0_BASE_ADDR 0x101f1000
#define UART0_DR (*((volatile uint32_t *)(UART0_BASE_ADDR + 0x000)))
#define UART0_IMSC (*((volatile uint32_t *)(UART0_BASE_ADDR + 0x038)))

#define VIC_BASE_ADDR 0x10140000
#define VIC_INTENABLE (*((volatile uint32_t *)(VIC_BASE_ADDR + 0x010)))

#include "io_serial.h"
#include "io_angel.h"

#include <stdio.h>

ATOSE_IO_angel io;

volatile long *ticks = (long *)((unsigned char *)0x101E2000 + 0x04);
volatile long happened = 0;
volatile long global_sp = 0;

/*
	Programmable Interrupt controller
*/
unsigned char *PIC_base_address = (unsigned char *)0x10140000;

volatile unsigned long *PIC_IRQ_status_register = (unsigned long *)(PIC_base_address + 0x00);
volatile unsigned long *PIC_FIRQ_status_register = (unsigned long *)(PIC_base_address + 0x04);
volatile unsigned long *PIC_RAW_status_register = (unsigned long *)(PIC_base_address + 0x08);
volatile unsigned long *PIC_interrupt_select_register = (unsigned long *)(PIC_base_address + 0x0C);
volatile unsigned long *PIC_interrupt_enable_register = (unsigned long *)(PIC_base_address + 0x10);
volatile unsigned long *PIC_interrupt_enable_clear_register = (unsigned long *)(PIC_base_address + 0x14);
volatile unsigned long *PIC_software_interrupt_register = (unsigned long *)(PIC_base_address + 0x18);
volatile unsigned long *PIC_software_interrupt_clear_register = (unsigned long *)(PIC_base_address + 0x1C);
volatile unsigned long *PIC_protection_enable_register = (unsigned long *)(PIC_base_address + 0x20);
volatile unsigned long *PIC_vector_address_register = (unsigned long *)(PIC_base_address + 0x30);
volatile unsigned long *PIC_default_vector_address_register = (unsigned long *)(PIC_base_address + 0x34);
volatile unsigned long *PIC_vector_address_registers = (unsigned long *)(PIC_base_address + 0x100);
volatile unsigned long *PIC_vector_control_registers = (unsigned long *)(PIC_base_address + 0x200);
volatile unsigned long *PIC_peripheral_id_register = (unsigned long *)(PIC_base_address + 0xFE0);

/*
	Timer
*/
unsigned char *timer_base_address = (unsigned char *)0x101E2000;

volatile unsigned long *timer_0_Load = (unsigned long *)(timer_base_address + 0x00);
volatile unsigned long *timer_0_Value = (unsigned long *)(timer_base_address + 0x04);
volatile unsigned long *timer_0_Control = (unsigned long *)(timer_base_address + 0x08);
volatile unsigned long *timer_0_IntClr = (unsigned long *)(timer_base_address + 0x0C);
volatile unsigned long *timer_0_RIS = (unsigned long *)(timer_base_address + 0x10);
volatile unsigned long *timer_0_MIS = (unsigned long *)(timer_base_address + 0x14);
volatile unsigned long *timer_0_BGLoad = (unsigned long *)(timer_base_address + 0x18);

/*
	GET_CPSR()
	----------
*/
inline int get_cpsr(void)
{ 
int answer;

asm volatile ("mrs %0,CPSR":"=r" (answer));

return answer;
}

/*
	SET_CPSR()
	----------
*/
inline void set_cpsr(int save_cpsr)
{
asm volatile ("msr CPSR_cxsf, %0"::"r"(save_cpsr));
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
	GET_SP()
	--------
*/
inline unsigned int get_sp(void)
{
unsigned int answer;

asm volatile ("mov %0,sp":"=r" (answer));

return answer;
}

/*
	__CS3_ISR_IRQ()
	---------------
*/
volatile unsigned long isr;
extern "C" {
		void __attribute__ ((interrupt)) __cs3_isr_irq()
		{
		/*
			Read from the IRQ vector address register to signal to the interrupt controller
			that we are servicing the interrupt
		*/
		isr = *PIC_vector_address_register;

		happened++;

		/*
			Tell the timer that we've serviced the interrupt
		*/
		*timer_0_IntClr = 0;

		/*
			Tell the interrupt controller that we're done servicing the interrupt
			*/
		*PIC_vector_address_register = isr;
#ifdef NEVER
	UART0_DR = UART0_DR + 1;
#endif
		}
}


/*
	START_TIMER()
	-------------
*/
void start_timer(void)
{
unsigned long was;

io.hex();

/*
	Enable IRQ
*/
io << "enable IRQ" << ATOSE_IO::eoln;
enable_IRQ();

/*
	Enable the primary interrupt controller
*/
io << "Enable PIC" << ATOSE_IO::eoln;
*PIC_interrupt_enable_register = 0x10;

/*
	Timer
*/
io << "Enable timer" << ATOSE_IO::eoln;

*timer_0_Load = (unsigned long)0x8000;
was = *timer_0_Control;
was = was & 0xFFFFFF10 | 0xE0;
*timer_0_Control = was;

io << "Done" << ATOSE_IO::eoln;
}

/*
	ENABLE_STACKS()
	---------------
*/
void enable_stacks(void)
{
#define irq_stack_size 1024
static unsigned char irq_stack[irq_stack_size];
unsigned char *top_of_stack = irq_stack + sizeof(irq_stack);

/*
	Get the current mode in r0
*/
asm volatile
	(
	"mrs r0, cpsr \n"
	/*
		Switch the CPU into IRQ mode
	*/
	"bic r1, r0, #0x1F \n"			/* get the mode bits */
	"orr r1, r1, #0x12 \n"			/* turn on IRQ mode bits */
	"msr cpsr, r1 \n"				/* go into IRQ mode */
	/*																	
		Set the stack pointer											
	*/																	
	"mov sp, %[top_of_stack] \n"
	/*
		Go back into what ever mode we were in before (Supervisor mode)
	*/
	"msr cpsr, r0 \n"
	:
	: [top_of_stack]"r"(top_of_stack)
	: "r1", "r2", "cc"
	);
}

/*
	MAIN()
	------
*/
int main(void)
{
int x;

enable_stacks();
enable_IRQ();

start_timer();

for (x = 0; x < 100; x++)
	io << "Ticks:" << *ticks << " interrupts:" << happened << ATOSE_IO::eoln;
io << "Done";

/*

enable_IRQ();

io.puts("Enter");

VIC_INTENABLE = 1<<12;
UART0_IMSC = 1<<4;
for(;;);
	io << ".";
*/

return 0;
}


