/*
	ATOSE.C
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "stack.h"
#include "registers.h"

/*
	ATOSE_ATOSE::ATOSE_ATOSE()
	--------------------------
*/
ATOSE_atose::ATOSE_atose() : debug(imx6q_serial_port), cpu(imx6q_cpu), interrupt_controller(imx6q_gic) 	//, usb(imx6q_usb)
{
set_ATOSE();
}

/*
	ATOSE_ATOSE::RESET()
	--------------------
*/
void ATOSE_atose::reset(ATOSE_registers *registers)
{
/*
	No need to set up the stacks because that was done as part of the object creation
	But we do need to set up the IRQ
*/
cpu.set_interrupt_handlers(this);
debug << "ATOSE up and running" << ATOSE_debug::eoln;

debug << "Enable USB...(disabled)...";

//usb.enable();
debug << "done" << ATOSE_debug::eoln;

debug << "Tell Interrupt controller about USB...(disabled)...";
//interrupt_controller.enable(&usb, usb.get_interrup_id());
debug << "done" << ATOSE_debug::eoln;

debug << "Tell Interrupt controller about USB HOST...";
interrupt_controller.enable(&imx6q_host_usb, imx6q_host_usb.get_interrup_id());
debug << "done" << ATOSE_debug::eoln;

debug << "enable IRQ";
cpu.enable_irq();
debug << "done" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_PREFETCH_ABORT()
	---------------------------------
*/
void ATOSE_atose::isr_prefetch_abort(ATOSE_registers *registers)
{
debug << "Prefetch Abort" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_DATA_ABORT()
	-----------------------------
*/
void ATOSE_atose::isr_data_abort(ATOSE_registers *registers)
{
debug << "Data Abort" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_UNDEFINED()
	----------------------------
*/
void ATOSE_atose::isr_undefined(ATOSE_registers *registers)
{
debug << "Undefined Instruction" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_RESERVED()
	---------------------------
	As this can't happen (there is no such thing as the reserved interrupt) we don't need to worry about it.
*/
void ATOSE_atose::isr_reserved(ATOSE_registers *registers)
{
debug << "Reserved Interrupt (it should not be possible for this to happen" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_FIRQ()
	-----------------------
*/
void ATOSE_atose::isr_firq(ATOSE_registers *registers)
{
debug << "FIRQ" << ATOSE_debug::eoln;
}

/*
	ATOSE_ATOSE::ISR_SWI()
	----------------------
*/
void ATOSE_atose::isr_swi(ATOSE_registers *registers)
{
debug << "SWI" << ATOSE_debug::eoln;
}

