.global _Reset
.global ATOSE_vectors_start
.global ATOSE_vectors_end

ATOSE_vectors_start:
_Reset:
 LDR PC, reset_handler_addr
 LDR PC, undef_handler_addr
 LDR PC, swi_handler_addr
 LDR PC, prefetch_abort_handler_addr
 LDR PC, data_abort_handler_addr
 LDR PC, reserved_handler_addr
 LDR PC, irq_handler_addr
 LDR PC, fiq_handler_addr

reset_handler_addr: .word reset_handler
undef_handler_addr: .word __cs3_isr_undef
swi_handler_addr: .word __cs3_isr_swi
prefetch_abort_handler_addr: .word  __cs3_isr_pabort
data_abort_handler_addr: .word __cs3_isr_dabort
reserved_handler_addr: .word __cs3_isr_reserved
irq_handler_addr: .word __cs3_isr_irq
fiq_handler_addr: .word __cs3_isr_fiq

ATOSE_vectors_end:

reset_handler:
  LDR sp, =stack_top
  BL c_entry
  B .

