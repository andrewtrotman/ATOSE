/*
	CPU_ARM_IMX6Q.C
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>
#include <string.h>

#include "cpu_arm_imx6q.h"
#include "interrupt_arm.h"

/*
	ATOSE_CPU_ARM_IMX6Q::SET_INTERRUPT_HANDLERS()
	---------------------------------------------
	OK, so, here we go...  On the i.MX6Q the interrupt vectors are in ROM. This might not be so bad except that the alternative location (high mem)
	might not exist because its in external RAM which might not go that high.  To get around this Freescale's ROM vectors point to a location in
	on-chip RAM (0x0093FFB8). More precisely, when an IRQ happens the CPU jumps to the IRQ vector (0x00000018) which does a branch indirect to the
	address stored in 0x0000003C which results in a branch to 0x0093FFD0 which is in the on-chip RAM.  What happens there is another branch indirect
	to the address stored in 0x93FFF4.  That, in turn,  takes the CPU to location 0x0000F7F4.

	To take control of the interrupts its necessary to either change the branch table at 0x0093FFB8, or the vector of addresses at 0x0093FFDC

	0x00000000
		1C F0 9F E5 ldr pc, LOW_RESET
		1C F0 9F E5 ldr pc, LOW_UNDEF
		1C F0 9F E5 ldr pc, LOW_SWI
		1C F0 9F E5 ldr pc, LOW_PREFETCH_ABORT
		1C F0 9F E5 ldr pc, LOW_DATA_ABORT
		1C F0 9F E5 ldr pc, LOW_RESERVED
		1C F0 9F E5 ldr pc, LOW_IRQ
		1C F0 9F E5 ldr pc, LOW_FIRQ
		1C F0 9F E5 ldr pc, LOW_SW_MONITOR
	0x00000024
		LOW_RESET:
			2C F8 00 00
		LOW_UNDEF:
			BC FF 93 00
		LOW_SWI:
			C0 FF 93 00
		LOW_PREFETCH ABORT:
			C4 FF 93 00
		LOW_DATA_ABORT:
			C8 FF 93 00
		LOW_RESERVED:
			CC FF 93 00
		LOW_IRQ:
			D0 FF 93 00
		LOW_FIRQ:
			D4 FF 93 00
		LOW_SW_MONITOR:
			D8 FF 93 00

	0x0093FFB8:
		1C F0 9F E5 ldr pc, HI_RESET			; note, this cannot get called
		1C F0 9F E5 ldr pc, HI_UNDEF
		1C F0 9F E5 ldr pc, HI_SWI
		1C F0 9F E5 ldr pc, HI_PREFETCH ABORT
		1C F0 9F E5 ldr pc, HI_DATA ABORT
		1C F0 9F E5 ldr pc, HI_RESERVED
		1C F0 9F E5 ldr pc, HI_IRQ
		1C F0 9F E5 ldr pc, HI_FIRQ
		1C F0 9F E5 ldr pc, HI_SW_MONITOR
	0x0093FFDC:
		HI_RESET:
			2C F8 00 00 RESET
		HI_UNDEF:
			F4 F7 00 00 UNDEF
		HI_SWI:
			F4 F7 00 00 SWI
		HI_PREFETCH_ABORT:
			F4 F7 00 00 PREFETCH_ABORT
		HI_DATA_ABORT:
			F4 F7 00 00 DATA_ABORT
		HI_RESERVED:
			F4 F7 00 00 RESERVED
		HI_IRQ:
			F4 F7 00 00 IRQ
		HI_FIRQ:
			F4 F7 00 00 FIRQ
		HI_SW_MONITOR:
			F4 F7 00 00 SW_MONITOR	; Software Watchdog

	See pages 397, 400 and 401 of "i.MX 6Dual/6Quad Applications Processor Reference Manual, Rev. 0, 11/2012" for a memory map of the on-chip RAM and
	a somewhat brief explanation of what is going on (but my explanation is better).
*/
void ATOSE_cpu_arm_imx6q::set_interrupt_handlers(void *atose_object)
{
extern uint8_t ATOSE_interrupt_vectors_start;
extern uint8_t ATOSE_interrupt_vectors_finish;

memcpy((void *)ARM_INTERRUPT_VECTOR_TABLE_ADDRESS, &ATOSE_interrupt_vectors_start, ATOSE_interrupt_vectors_finish - ATOSE_interrupt_vectors_start);
((ATOSE_interrupt_arm *)ARM_INTERRUPT_VECTOR_TABLE_ADDRESS)->reserved = atose_object;
}
