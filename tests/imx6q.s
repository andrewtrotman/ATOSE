.section .ATOSE_entry_point
.global _Reset

.weak __cs3_isr_undef
.weak __cs3_isr_pabort
.weak __cs3_isr_dabort
.weak __cs3_isr_reserved
.weak __cs3_isr_fiq
.weak ATOSE_isr_irq
.weak ATOSE_isr_swi

.weak abort
.weak _sbrk_r
.weak _kill_r
.weak _getpid_r
abort:
_sbrk_r:
_kill_r:
_getpid_r:

ATOSE_vectors_start:

/*
	RESET_HANDLER
	-------------
*/
_Reset:
	ldr sp, =stack_top
	b main
