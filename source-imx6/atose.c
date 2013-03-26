/*
	ATOSE.C
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "idle.h"
#include "pipe.h"
#include "atose.h"
#include "stack.h"
#include "registers.h"
#include "atose_api.h"
#include "semaphore.h"
#include "process_allocator.h"

void debug_print_string(const char *string);

/*
	ATOSE_ATOSE::ATOSE_ATOSE()
	--------------------------
*/
ATOSE_atose::ATOSE_atose() : imx6q_heap(), process_allocator(&imx6q_heap), scheduler(&imx6q_heap, &process_allocator), process_clock(imx6q_process_clock), heap(imx6q_heap), debug(imx6q_serial_port), cpu(imx6q_cpu), interrupt_controller(imx6q_gic) /*, usb(imx6q_usb) */
{
set_ATOSE();
}

/*
	START_SHELL()
	-------------
*/
uint32_t start_shell(void)
{
while (ATOSE_atose::get_ATOSE()->file_system.isdead())
	;	// do nothing

ATOSE_api::writeline("\r\nStart SHELL\r\n");
ATOSE_api::spawn("SHELL.ELF");
ATOSE_api::writeline("BACK\r\n");

ATOSE_api::exit(0);

return 0;
}

/*
	ATOSE_ATOSE::RESET()
	--------------------
*/
void ATOSE_atose::reset(ATOSE_registers *registers)
{
ATOSE_api api;

/*
	No need to set up the stacks because that was done as part of the object creation
	But we do need to set up the IRQ
*/
cpu.set_interrupt_handlers(this);
debug << "\033[1;1H\033[40;1;37m\033[2J";
debug << "ATOSE version i" << ATOSE_debug::eoln;
//debug << "ATOSE SIZE:" << sizeof(ATOSE_atose) << "\r\n";

heap.init();		// and turn on the MMU

//usb.enable();
//interrupt_controller.enable(&usb, usb.get_interrup_id());

interrupt_controller.enable(&process_clock, process_clock.get_interrup_id());
process_clock.enable();

//debug  << "Start the IDLE process";
scheduler.create_system_thread(idle, true);
//debug << "done" << ATOSE_debug::eoln;

interrupt_controller.enable(&imx6q_host_usb, imx6q_host_usb.get_interrup_id());
imx6q_host_usb.enable();

//scheduler.create_system_thread(start_shell);

void pipe_test(void);
pipe_test();


/*
	Now we've bootstrapped ATIRE, we start processing
*/
//debug << "Enable IRQ" << ATOSE_debug::eoln;
cpu.enable_irq();

while (1)
	;	// this will never happen because once we enable IRQ we'll get an event to switch processes
}

/*
	ATOSE_ATOSE::ISR_PREFETCH_ABORT()
	---------------------------------
*/
void ATOSE_atose::isr_prefetch_abort(ATOSE_registers *registers)
{
debug.hex();
debug << "Prefetch Abort at address:0x" << (registers->r14_current - 4) << ATOSE_debug::eoln;
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










void ATOSE_putc(ATOSE_registers *registers);
void ATOSE_getc(ATOSE_registers *registers);
void ATOSE_peekc(ATOSE_registers *registers);
void ATOSE_spawn(ATOSE_registers *registers);
void ATOSE_exit(ATOSE_registers *registers);
void ATOSE_semaphore_create(ATOSE_registers *registers);
void ATOSE_semaphore_clear(ATOSE_registers *registers);
void ATOSE_semaphore_signal(ATOSE_registers *registers);
void ATOSE_semaphore_wait(ATOSE_registers *registers);
void ATOSE_pipe_create(ATOSE_registers *registers);
void ATOSE_pipe_bind(ATOSE_registers *registers);
void ATOSE_pipe_connect(ATOSE_registers *registers);
void ATOSE_pipe_close(ATOSE_registers *registers);
void ATOSE_pipe_send(ATOSE_registers *registers);
void ATOSE_pipe_receive(ATOSE_registers *registers);
void ATOSE_pipe_reply(ATOSE_registers *registers);

typedef void(*ATOSE_system_method)(ATOSE_registers *);
ATOSE_system_method ATOSE_call[] =
{
ATOSE_putc,
ATOSE_getc,
ATOSE_peekc,
ATOSE_spawn,
ATOSE_exit,
ATOSE_semaphore_create,
ATOSE_semaphore_clear,
ATOSE_semaphore_signal,
ATOSE_semaphore_wait,
ATOSE_pipe_create,
ATOSE_pipe_bind,
ATOSE_pipe_connect,
ATOSE_pipe_close,
ATOSE_pipe_send,
ATOSE_pipe_receive,
ATOSE_pipe_reply
};

/*
	ATOSE_ATOSE::ISR_SWI()
	----------------------
*/
void ATOSE_atose::isr_swi(ATOSE_registers *registers)
{
/*
	Save the state of the registers
*/
memcpy(&ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->execution_path->registers, registers, sizeof(*registers));

/*
	First we need to determine whether or no the SWI is for us.  We do this by getting the SWI number,
	that number is stored in the instruction just executed, which is stored at R14.  So we subtract 4 from
	R14 to get the instruction then turn off the top bits to get the number

	If it is for us we validate the method id.  If that's valid we call the method
*/
if (( (*(uint32_t *)(registers->r14_current - 4)) & 0x00FFFFFF) == ATOSE_SWI)
	if (registers->r0 < ATOSE_END_OF_METHODS)
		ATOSE_call[registers->r0](&ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->execution_path->registers);

/*
	Context switch
*/
ATOSE_atose::get_ATOSE()->scheduler.context_switch(registers);
}

/*
	ATOSE_PUTC()
	------------
*/
void ATOSE_putc(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->debug.write_byte(registers->r1);
}

/*
	ATOSE_GETC()
	------------
	return the next character from the input buffer
*/
void ATOSE_getc(ATOSE_registers *registers)
{
uint8_t answer;

ATOSE_atose::get_ATOSE()->debug.read_byte(&answer);
registers->r0 = answer;
}

/*
	ATOSE_PEEKC()
	-------------
	return the number of characters known to be in the input buffer
*/
void ATOSE_peekc(ATOSE_registers *registers)
{
registers->r0 = ATOSE_atose::get_ATOSE()->debug.peek();
}

/*
	ATOSE_SPAWN()
	-------------
	r1 = filename to execute
*/
void ATOSE_spawn(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->heap.assume_identity();
if (ATOSE_atose::get_ATOSE()->scheduler.create_process((uint8_t *)registers->r1) == ATOSE_process_manager::SUCCESS)
	ATOSE_atose::get_ATOSE()->heap.assume(ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->address_space);
}

/*
	ATOSE_EXIT()
	------------
*/
void ATOSE_exit(ATOSE_registers *registers)
{
ATOSE_atose::get_ATOSE()->debug << "EXIT()";
ATOSE_atose::get_ATOSE()->scheduler.terminate_current_process();
}

/*
	ATOSE_SEMAPHORE_CREATE()
	------------------------
*/
void ATOSE_semaphore_create(ATOSE_registers *registers)
{
ATOSE_semaphore *semaphore;

semaphore = ATOSE_atose::get_ATOSE()->process_allocator.malloc_semaphore();
semaphore->clear();

registers->r0 = (uint32_t)semaphore;
}

/*
	ATOSE_SEMAPHORE_CLEAR()
	-----------------------
*/
void ATOSE_semaphore_clear(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->clear();
}

/*
	ATOSE_SEMAPHORE_SIGNAL()
	------------------------
*/
void ATOSE_semaphore_signal(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->signal();
}

/*
	ATOSE_SEMAPHORE_WAIT()
	----------------------
*/
void ATOSE_semaphore_wait(ATOSE_registers *registers)
{
((ATOSE_semaphore *)registers->r1)->wait();
}

/*
	ATOSE_PIPE_CREATE()
	-------------------
*/
void ATOSE_pipe_create(ATOSE_registers *registers)
{
ATOSE_pipe *pipe;

pipe = ATOSE_atose::get_ATOSE()->process_allocator.malloc_pipe();
pipe->initialise();
registers->r0 = (uint32_t)pipe;
}

/*
	ATOSE_PIPE_BIND()
	-----------------
*/
void ATOSE_pipe_bind(ATOSE_registers *registers)
{
registers->r0 = ((ATOSE_pipe *)registers->r1)->bind(registers->r2);
}

/*
	ATOSE_PIPE_CONNECT()
	--------------------
*/
void ATOSE_pipe_connect(ATOSE_registers *registers)
{
registers->r0 = ((ATOSE_pipe *)registers->r1)->connect(registers->r2);
}

/*
	ATOSE_PIPE_CLOSE()
	------------------
*/
void ATOSE_pipe_close(ATOSE_registers *registers)
{
registers->r0 = ((ATOSE_pipe *)registers->r1)->close();
ATOSE_atose::get_ATOSE()->process_allocator.free((ATOSE_pipe *)registers->r1);
}

/*
	ATOSE_PIPE_SEND()
	-----------------
*/
void ATOSE_pipe_send(ATOSE_registers *registers)
{
((ATOSE_pipe *)registers->r1)->send((void *)registers->r2, registers->r3, (void *)registers->r4, registers->r5);
}

/*
	ATOSE_PIPE_RECEIVE()
	--------------------
*/
void ATOSE_pipe_receive(ATOSE_registers *registers)
{
((ATOSE_pipe *)registers->r1)->receive((void *)registers->r2, registers->r3);
}

/*
	ATOSE_PIPE_REPLY()
	------------------
*/
void ATOSE_pipe_reply(ATOSE_registers *registers)
{
((ATOSE_pipe_task*)registers->r1)->server->reply(registers->r1, (void *)registers->r2, registers->r3);
}
