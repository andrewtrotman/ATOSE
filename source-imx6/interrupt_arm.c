/*
	INTERRUPT_ARM.C
	---------------
*/
#include <stdint.h>

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
	CPU_INTERRUPT_INIT()
	--------------------
	OK, so, here we go...  On the i.MX6Q the interrupt vectors are in ROM. This might not be so bad except that the alternative location (high mem)
	might not exist because its in external RAM which might not go that high.  To get around this Freescale's ROM vectors point to a location in
	on-chip RAM (0x0093FFB8). More precisely, when an IRQ happens the CPU jumps to the IRQ vector (0x00000018) which does a branch indirect to the
	address stored in 0x0000003C which results in a branch to 0x0093FFD0 which is in the on-chip RAM.  What happens there is another branch indirect
	to the address stored in 0x93FFF4.  That, in turn,  takes the CPU to location 0x0000F7F4.

	To take control of the interrupts its necessary to either change the branch table at 0x0093FFB8, or the vector of addresses at 0x0093FFDC

	0x0093FFDC:
		2C F8 00 00 RESET
		F4 F7 00 00 UNDEF
		F4 F7 00 00 SWI
		F4 F7 00 00 PREFETCH ABORT
		F4 F7 00 00 DATA ABORT
		F4 F7 00 00 RESERVED
		F4 F7 00 00 IRQ
		F4 F7 00 00 FIRQ
		F4 F7 00 00 SW MONITOR	// Software Watchdog

	See pages 397, 400 and 401 of "i.MX 6Dual/6Quad Applications Processor Reference Manual, Rev. 0, 11/2012" for a memory map of the on-chip RAM and
	a somewhat brief explanation of what is going on.
*/
void cpu_interrupt_init(void)
{
ARM_interrupt_vectors *vectors = (ARM_interrupt_vectors *)0x0093FFDC;		// top of on-chip RAM

vectors->irq = (uint32_t)isr_IRQ;
enable_IRQ();
}

/*
   INTERRUPT_INIT()
   ----------------
*/
void interrupt_init(void)
{
/*
	Enable all interrupt levels (0 = high, 0xFF = low)
*/
cpu_registers->interrupt_priority_mask_register = 0xFF;

/*
   Enable the GIC (both secure and insecure interrupts)
*/
cpu_registers->cpu_interface_control_register = 0x03;					// enable everything
distributor_registers->distributor_control_register = 0x03;			// enable everything

/*
   Enable the GPT interrupt
*/
distributor_registers->interrupt_priority_registers[IMX_INT_GPT] = 0;												// highest priority
distributor_registers->interrupt_security_registers[IMX_INT_GPT / 32] &= ~(1 << (IMX_INT_GPT & 0x1F));	// disable security
distributor_registers->interrupt_processor_targets_registers[IMX_INT_GPT] |= 1;									// send to CPU 0
distributor_registers->interrupt_set_enable_registers[IMX_INT_GPT / 32] = 1 << (IMX_INT_GPT & 0x1F);		// enable the interrupt
}

/*
   MAIN()
   ------
*/
int main(void)
{
uint32_t irq_stack[256];
uint32_t *irq_sp = irq_stack + sizeof(irq_stack);

uint32_t base;
long x;
uint32_t count;


serial_init();
debug_puts("\r\nStart\r\nBuild Time:" __DATE__ ", " __TIME__ "\r\n");
timer_init();

/*
	Set up the IRQ stack
*/
asm volatile
(
		"mov r2, %[stack];"		// grab the stack address
		"mrs r0, cpsr;"			// get the current mode
		"bic r1, r0, #0x1F;"	// turn off the mode bits
		"orr r1, r1, #0x12;"	// turn on the IRQ bits
		"msr cpsr, r1;"			// go into IRQ mode
		"mov sp, r2;"			// set the stack top
		"msr cpsr, r0;"			// back to original mode
		:
		: [stack]"r"(irq_sp)
		: "r0", "r1", "r2"
);

HW_GPT_OCR1.U = 0x100;     // run until the GPT gets to 0x100
HW_GPT_CR.B.FRR = 0;    	// restart mode (periodic interrupt mode)

/*
	Get the address of the CPU's configuration registers, but in the address space of the SOC.
	This instruction only works on the Cortex-A9 MPCore, it does not work on a unicore Cortex A9.
	It has been tested on the i.MX6Q
*/
asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);

cpu_registers = (ARM_generic_interrupt_controller_cpu_register_map *)(base + 0x100);
distributor_registers = (ARM_generic_interrupt_controller_distributor_register_map *)(base + 0x1000);

/*
	On the Freescale i.MX6Q this should produce:00A00000
*/
debug_puts("ARM Registers Address:");
debug_print_hex(base);
debug_puts("\r\n");

/*
	On the Freescale i.MX6Q this should produce:3901243B
*/
debug_puts("cpu_interface_dentification_register:");
debug_print_hex(cpu_registers->cpu_interface_dentification_register);
debug_puts("\r\n");

/*
	On the Freescale i.MX6Q this should produce: 0000000D 000000F0 00000005 000000B1
*/
debug_puts("Component ID:");
debug_print_hex(distributor_registers->component_id0);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id1);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id2);
debug_puts(" ");
debug_print_hex(distributor_registers->component_id3);
debug_puts("\r\n");

/*
	Enable interrupts
*/
interrupt_init();
cpu_interrupt_init();
HW_GPT_IR.B.OF1IE = 1;

/*
	On to the program. We just print the value of the counter and the number
	of times its has wrapped over.
*/
for (x = 0; x < 10; x++)
   {
   count = HW_GPT_CNT_RD();

   debug_puts("GPT_CNT:");
   debug_print_hex(count);
   debug_puts(" ");

   debug_puts("IRQCount:");
   debug_print_hex(interrupt_count);
   debug_puts("\r\n");
   }

return 0;
}
