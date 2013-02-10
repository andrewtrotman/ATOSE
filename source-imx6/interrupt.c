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
	__CXA_PURE_VIRTUAL()
	--------------------
*/
extern "C" void __cxa_pure_virtual()
{
ATOSE_atose::get_ATOSE()->debug << "Pure Virtual Function Call" << ATOSE_debug::eoln;
}

/*
	ATOSE_ISR_UNDEF()
	-----------------
*/
extern "C" void ATOSE_isr_undef(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_undefined(registers);
}

/*
	ATOSE_ISR_PABORT()
	------------------
*/
extern "C" void ATOSE_isr_pabort(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_prefetch_abort(registers);
}

/*
	ATOSE_ISR_DABORT()
	------------------
*/
extern "C" void ATOSE_isr_dabort(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_data_abort(registers);
}

/*
	ATOSE_ISR_RESERVED()
	--------------------
*/
extern "C" void ATOSE_isr_reserved(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_reserved(registers);
}

/*
	ATOSE_ISR_FIRQ()
	----------------
*/
extern "C" void ATOSE_isr_firq(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_firq(registers);
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
extern "C" void ATOSE_isr_irq(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_irq(registers);
}

/*
	ATOSE_ISR_SWI()
	---------------
*/
extern "C" void ATOSE_isr_swi(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->isr_swi(registers);
}
