/*
	INTERRUPT.C
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	When an interrupt occurs it gets caught by the assembly and eventually passed through to here.
*/
#include "atose.h"
#include "registers.h"

/*
	ATOSE_GET_ATOSE()
	-----------------
*/
static inline ATOSE_atose *ATOSE_get_atose(void)
{
extern uint32_t ATOSE_pointer;
ATOSE_atose *object;

object = *((ATOSE_atose *)(&ATOSE_pointer));

return object;
}

/*
	ATOSE_ISR_UNDEF()
	-----------------
*/
void ATOSE_isr_undef(ATOSE_registers *registers)
{
ATOSE_get_atose()->isr_undefined();
}

/*
	ATOSE_ISR_PABORT()
	------------------
*/
void ATOSE_isr_pabort(ATOSE_registers *registers)
{
ATOSE_get_atose()->isr_prefetch_abort();
}

/*
	ATOSE_ISR_DABORT()
	------------------
*/
void ATOSE_isr_dabort(ATOSE_registers *registers)
{
ATOSE_get_atose()->isr_data_abort();
}

/*
	ATOSE_ISR_RESERVED()
	--------------------
*/
void ATOSE_isr_reserved(ATOSE_registers *registers)
{
ATOSE_get_atose()->isr_reserved();
}

/*
	ATOSE_ISR_FIQ()
	---------------
*/
void ATOSE_isr_fiq()
{
ATOSE_get_atose()->isr_fiq(ATOSE_registers *registers);
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
void ATOSE_isr_irq()
{
ATOSE_get_atose()->isr_irq(ATOSE_registers *registers);
}

/*
	ATOSE_ISR_SWI()
	---------------
*/
void ATOSE_isr_swi()
{
ATOSE_get_atose()->isr_swi(ATOSE_registers *registers);
}
