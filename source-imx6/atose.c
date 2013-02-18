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
ATOSE_atose::ATOSE_atose() : imx6q_heap(), scheduler(&imx6q_heap), process_clock(imx6q_process_clock), heap(imx6q_heap), debug(imx6q_serial_port), cpu(imx6q_cpu), interrupt_controller(imx6q_gic) /*, usb(imx6q_usb) */
{
set_ATOSE();
}

/*
	IDLE()
	------
	This is the ATOSE idle process
*/
int idle(void)
{
uint32_t param, answer;

#define ATOSE_SWI 0x6174

while (1)
	{
	for (param = 0; param < 0xFFFF; param++)
		{
		asm volatile 
			(
			"mov r0, %[param];"
			"swi %[ATOSE_swi];"
			"mov %[answer], r0;"
			: [answer]"=r" (answer)
			: [param]"r"(param), [ATOSE_swi]"i"(ATOSE_SWI)
			: "r0"
			);
		}
	}

return 0;
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
//interrupt_controller.enable(&usb, usb.get_interrup_id());
debug << "done" << ATOSE_debug::eoln;

debug << "Tell Interrupt controller about TIMER...";
interrupt_controller.enable(&process_clock, process_clock.get_interrup_id());
process_clock.enable();
debug << "done" << ATOSE_debug::eoln;


debug << "Tell Interrupt controller about USB HOST...";
//interrupt_controller.enable(&imx6q_host_usb, imx6q_host_usb.get_interrup_id());
debug << "done" << ATOSE_debug::eoln;

debug << "enable IRQ...";
cpu.enable_irq();
debug << "done" << ATOSE_debug::eoln;

debug  << "Start the IDLE process";
heap.init();
scheduler.create_idle_process(idle);
debug << "done" << ATOSE_debug::eoln;

while (1)
	;	/* nothing */
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
while (1);
}

/*
	ATOSE_ATOSE::ISR_UNDEFINED()
	----------------------------
*/
void ATOSE_atose::isr_undefined(ATOSE_registers *registers)
{
debug.hex();
debug << "Undefined Instruction at address:0x" << (registers->r14_current - 4) << ATOSE_debug::eoln;
while (1)
	;	/* hang */
}

/*
	ATOSE_ATOSE::ISR_RESERVED()
	---------------------------
	As this can't happen (there is no such thing as the reserved interrupt) we don't need to worry about it.
*/
void ATOSE_atose::isr_reserved(ATOSE_registers *registers)
{
debug << "Reserved Interrupt (it should not be possible for this to happen)" << ATOSE_debug::eoln;
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

