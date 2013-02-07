/*
	ATOSE_STARTUP.ASM
	------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This file manages the bootstrapping between ASM and C++
*/

/*
	We define two globals, one is the program entry point, the other is
	the address in memory in which the ATOSE object is kept.  This second
	one is needed so that it can be captured by the interrupt handler and
	boot-strapped into.
*/
.global _Reset
.global ATOSE_pointer
/*
	Make the pointes to the interrupt table global too
*/
.global ATOSE_interrupt_vectors_start
.global ATOSE_interrupt_vectors_finish


/*
	The addresses of the interrupt handlers
*/
.weak ATOSE_isr_undef, ATOSE_isr_pabort, ATOSE_isr_dabort, ATOSE_isr_reserved, ATOSE_isr_fiq, ATOSE_isr_irq, ATOSE_isr_swi

/*
	Stuff the compuler insists on
*/
.weak abort, _sbrk_r, _kill_r, _getpid_r
abort:
_sbrk_r:
_kill_r:
_getpid_r:

/*
	ENTRY POINT
	-----------
*/
_Reset:
	ldr pc, reset_handler_addr

/*
	The run-time interrupt vector table that gets copied into the right location in RAM
*/
ATOSE_interrupt_vectors_start:
reset_handler_addr:
	.word reset_handler
undef_handler_addr:
	.word undef_handler
swi_handler_addr:
	.word swi_handler
prefetch_abort_handler_addr:
	.word pabort_handler
data_abort_handler_addr:
	.word dabort_handler
ATOSE_pointer:
reserved_handler_addr:
	.word reserved_handler
irq_handler_addr:
	.word irq_handler
firq_handler_addr:
	.word firq_handler
ATOSE_interrupt_vectors_finish:

/*
	INTERRUPT_PREAMBLE
	------------------
*/
.macro interrupt_preamble
	/*
		push my link register (R14) on the stack then with all the user mode registers
	*/
	stmfd sp!, {r14}			/* push my R14 */
	stmfd sp, {r0-r14}^			/* save the user mode registers (cannot do swtfd! because it confuses the two r13s) */
	nop							/* and no op */
	sub sp, sp, #60				/* shift the stack pointer 15 * 4 bytes */

	/*
		Save the flags
	*/
	mrs r0, spsr
	stmfd sp!, {r0}

.endm

/*
	INTERRUPT_POSTAMBLE
	-------------------
*/
.macro interrupt_postamble
	/*
		get the flags
	*/
	ldmfd sp!, {r0}
	msr spsr, r0

	/*
		Restore the user mode registers and my r14
	*/
	ldmfd sp, {r0-r14}^			/* restore the user mode registers */
	nop							/* and no op */
	add sp, sp, #60				/* return my own stack pointer */
	ldmfd sp!, {r14}			/* get my R14 */
.endm

/*
	RTI_IRQ
	-------
*/
.macro rti_irq
	subs pc, r14, #4
.endm

/*
	RTI_FIRQ
	--------
*/
.macro rti_firq
	subs pc, r14, #4
.endm

/*
	RTI_SWI
	-------
*/
.macro rti_swi
	movs pc, r14
.endm

/*
	RTI_DABORT
	----------
	this will re-execute the instruction that caused the data abort.  To execute the instruciton after that do
	subs pc, r14, #4
*/
.macro rti_dabort
	subs pc, r14, #8
.endm

/*
	RTI_PABORT
	----------
*/
.macro rti_pabort
	subs pc, r14, #4
.endm

/*
	RTI_UNDEF
	---------
*/
.macro rti_undef
	movs pc, r14
.endm

/*
	RTI_RESERVED
	------------
	this is meaningless as this cannot happen because the interrupt cannot happen
*/
.macro rti_reserved
	movs pc, r14
.endm


/*
	RESET_HANDLER
	-------------
*/
reset_handler:
	ldr sp, =ATOSE_stack_top
	bl main
	wfi
	b .

/*
	IRQ_HANDLER
	-----------
*/
irq_handler:
	/*
		Shove the registers on the stack
	*/
	interrupt_preamble

	/*
		get a pointer to the registers and pass that to the interrupt handler
	*/
	mov r0, sp
	bl  ATOSE_isr_irq

	/*
		clean up the stack and return
	*/
	interrupt_postamble
	rti_irq

/*
	SWI_HANDLER
	-----------
	See the description of irq_handler for what is going on here.
*/
swi_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_swi
	interrupt_postamble
	rti_swi

/*
	UNDEF_HANDLER
	-------------
	See the description of irq_handler for what is going on here.
*/
undef_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_undef
	interrupt_postamble
	rti_undef

/*
	PABORT_HANDLER
	--------------
	See the description of irq_handler for what is going on here.
*/
pabort_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_pabort
	interrupt_postamble
	rti_pabort

/*
	DABORT_HANDLER
	--------------
	See the description of irq_handler for what is going on here.
*/
dabort_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_dabort
	interrupt_postamble
	rti_dabort

/*
	RESERVED_HANDLER
	----------------
	See the description of irq_handler for what is going on here.
	this is meaningless as this cannot happen because the interrupt cannot happen
*/
reserved_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_reserved
	interrupt_postamble
	rti_reserved

/*
	FIRQ_HANDLER
	------------
	See the description of irq_handler for what is going on here.
*/

firq_handler:
	interrupt_preamble
	mov r0, sp
	bl ATOSE_isr_firq
	interrupt_postamble
	rti_firq


