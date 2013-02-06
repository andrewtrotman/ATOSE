/*
	CPU_ARM_IMX6Q.C
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>

#include "cpu_arm_imx6q.h"
#include "interrupt_arm.h"

/*
	ATOSE_CPU_ARM_IMX6Q::SET_IRQ_HANDLER()
	--------------------------------------
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
void ATOSE_cpu_arm_imx6q::set_irq_handler(void *irq_handler)
{
ATOSE_interrupt_arm *vectors = (ATOSE_interrupt_arm *)0x0093FFDC;		// top of on-chip RAM

vectors->irq = irq_handler;
}
