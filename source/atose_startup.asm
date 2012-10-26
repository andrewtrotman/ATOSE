.global _Reset
.global ATOSE_vectors_start
.global ATOSE_vectors_end
.global ATOSE_addr

.weak __cs3_isr_undef
.weak __cs3_isr_pabort
.weak __cs3_isr_dabort
.weak __cs3_isr_reserved
.weak __cs3_isr_fiq
.weak ATOSE_isr_irq
.weak ATOSE_isr_swi

.weakref main, c_entry
.weak c_entry;

.weak abort
.weak _sbrk_r
.weak _kill_r
.weak _getpid_r
abort:
_sbrk_r:
_kill_r:
_getpid_r:

ATOSE_vectors_start:

_Reset:
	ldr pc, reset_handler_addr
	ldr pc, undef_handler_addr
	ldr pc, swi_handler_addr
	ldr pc, prefetch_abort_handler_addr
	ldr pc, data_abort_handler_addr
	ldr pc, reserved_handler_addr
	ldr pc, irq_handler_addr
	ldr pc, fiq_handler_addr

reset_handler_addr: .word reset_handler
undef_handler_addr: .word __cs3_isr_undef
swi_handler_addr: .word swi_handler
prefetch_abort_handler_addr: .word  __cs3_isr_pabort
data_abort_handler_addr: .word __cs3_isr_dabort
reserved_handler_addr: .word __cs3_isr_reserved
irq_handler_addr: .word irq_handler
fiq_handler_addr: .word __cs3_isr_fiq

ATOSE_vectors_end:

ATOSE_addr: .word 0

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
	RTI_SWI
	-------
*/
.macro rti_swi
	movs pc, r14
.endm


/*
	RESET_HANDLER
	-------------
*/
reset_handler:
	ldr sp, =stack_top
	bl c_entry
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
*/
swi_handler:
	/*
		Shove the registers on the stack
	*/
	interrupt_preamble

	/*
		Call the C/C++ interrupt handler
		Put a pointer to the on-stack registers into r0 as this is the first parameter to ATOSE_isr_swi.
		note that because it has a pointer to the registers on the stack (which are pulled on return) it can
		cause a return of multiple values by simply changing those values
	*/
	mov r0, sp
	bl ATOSE_isr_swi

	/*
		Restore the registers and return
	*/
	interrupt_postamble
	rti_swi

