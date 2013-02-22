/*
	ATOSE.C
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "idle.h"
#include "atose.h"
#include "stack.h"
#include "registers.h"
#include "atose_api.h"
#include "semaphore.h"
#include "process_allocator.h"

/*
	ATOSE_ATOSE::ATOSE_ATOSE()
	--------------------------
*/
ATOSE_atose::ATOSE_atose() : imx6q_heap(), process_allocator(&imx6q_heap), scheduler(&imx6q_heap, &process_allocator), process_clock(imx6q_process_clock), heap(imx6q_heap), debug(imx6q_serial_port), cpu(imx6q_cpu), interrupt_controller(imx6q_gic) /*, usb(imx6q_usb) */
{
set_ATOSE();
}

/*
	ATOSE_ATOSE::RESET()
	--------------------
*/
void ATOSE_atose::reset(ATOSE_registers *registers)
{
ATOSE_api api;

//debug << "ATOSE SIZE:" << sizeof(ATOSE_atose) << "\r\n";
/*
	No need to set up the stacks because that was done as part of the object creation
	But we do need to set up the IRQ
*/
cpu.set_interrupt_handlers(this);
debug << "\033[1;1H\033[40;1;37m\033[2J";
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
interrupt_controller.enable(&imx6q_host_usb, imx6q_host_usb.get_interrup_id());
imx6q_host_usb.enable();
debug << "done" << ATOSE_debug::eoln;

debug << "enable IRQ...";
cpu.enable_irq();
debug << "done" << ATOSE_debug::eoln;

debug  << "Start the IDLE process";
heap.init();
//scheduler.create_system_thread(idle);
debug << "done" << ATOSE_debug::eoln;


debug << "Wait for startup" << ATOSE_debug::eoln;
while (1)
	debug << ".";
}

/*
	ATOSE_ATOSE::ISR_PREFETCH_ABORT()
	---------------------------------
*/
void ATOSE_atose::isr_prefetch_abort(ATOSE_registers *registers)
{
debug << "Prefetch Abort" << ATOSE_debug::eoln;
while (1)
	;	/* hang */
}

/*
	ATOSE_ATOSE::ISR_DATA_ABORT()
	-----------------------------
*/
void ATOSE_atose::isr_data_abort(ATOSE_registers *registers)
{
debug.hex();
debug << "Data Abort at address:0x" << (registers->r14_current - 8) << ATOSE_debug::eoln;
while (1)
	;	/* hang */
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
while (1)
	;	/* hang */
}

/*
	ATOSE_ATOSE::ISR_FIRQ()
	-----------------------
*/
void ATOSE_atose::isr_firq(ATOSE_registers *registers)
{
debug << "FIRQ" << ATOSE_debug::eoln;
while (1)
	;	/* hang */
}










uint32_t ATOSE_putc(ATOSE_registers *registers);
uint32_t ATOSE_spawn(ATOSE_registers *registers);
uint32_t ATOSE_semaphore_create(ATOSE_registers *registers);
uint32_t ATOSE_semaphore_clear(ATOSE_registers *registers);
uint32_t ATOSE_semaphore_signal(ATOSE_registers *registers);
uint32_t ATOSE_semaphore_wait(ATOSE_registers *registers);

typedef uint32_t(*ATOSE_system_method)(ATOSE_registers *);
ATOSE_system_method ATOSE_call[] =
{
ATOSE_putc,
ATOSE_spawn,
ATOSE_semaphore_create,
ATOSE_semaphore_clear,
ATOSE_semaphore_signal,
ATOSE_semaphore_wait
};

/*
	ATOSE_ATOSE::ISR_SWI()
	----------------------
*/
uint32_t ATOSE_atose::isr_swi(ATOSE_registers *registers)
{
/*
	First we need to determine whether or no the SWI is for us.  We do this by getting the SWI number,
	that number is stored in the instruction just executed, which is stored at R14.  So we subtract 4 from
	R14 to get the instruction then turn off the top bits to get the number
*/
if (( (*(uint32_t *)(registers->r14_current - 4)) & 0x00FFFFFF) != ATOSE_SWI)
	return 0;

/*
	Illegal function call
*/
if (registers->r0 > ATOSE_END_OF_METHODS)
	return 0;

/*
	Dispatch
*/
return (ATOSE_call[registers->r0])(registers);
}


/*
	ATOSE_PUTC()
	------------
*/
uint32_t ATOSE_putc(ATOSE_registers *registers)
{
return ATOSE_atose::get_ATOSE()->debug.write_byte(registers->r1);
}

/*
	ATOSE_SPAWN()
	-------------
*/
uint32_t ATOSE_spawn(ATOSE_registers *registers)
{
uint32_t answer;

ATOSE_atose::get_ATOSE()->heap.assume_identity();
answer = ATOSE_atose::get_ATOSE()->scheduler.create_process((const uint8_t *)registers->r1, registers->r2);
ATOSE_atose::get_ATOSE()->heap.assume(ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->address_space);

return answer;
}

/*
	ATOSE_SEMAPHORE_CREATE()
	------------------------
*/
uint32_t ATOSE_semaphore_create(ATOSE_registers *registers)
{
ATOSE_semaphore *semaphore;

semaphore = ATOSE_atose::get_ATOSE()->process_allocator.malloc_semaphore();
semaphore->clear();

return registers->r0 = (uint32_t)semaphore;
}

/*
	ATOSE_SEMAPHORE_CLEAR()
	-----------------------
*/
uint32_t ATOSE_semaphore_clear(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->clear();

return 0;
}

/*
	ATOSE_SEMAPHORE_SIGNAL()
	------------------------
*/
uint32_t ATOSE_semaphore_signal(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->signal();

return 0;
}

/*
	ATOSE_SEMAPHORE_WAIT()
	----------------------
*/
uint32_t ATOSE_semaphore_wait(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->wait(registers);

return 0;
}

