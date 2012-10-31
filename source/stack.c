/*
	STACK.C
	-------
	asm(code : output operand list : input operand list : clobber list);
*/
#include "stack.h"

unsigned char ATOSE_stack::FIRQ_stack[STACK_SIZE_FIRQ] 			__attribute__ ((aligned (4)));
unsigned char ATOSE_stack::IRQ_stack[STACK_SIZE_IRQ] 				__attribute__ ((aligned (4)));
unsigned char ATOSE_stack::supervisor_stack[STACK_SIZE_SUPERVISOR] __attribute__ ((aligned (4)));
unsigned char ATOSE_stack::abort_stack[STACK_SIZE_ABORT] 			__attribute__ ((aligned (4)));
unsigned char ATOSE_stack::system_stack[STACK_SIZE_SYSTEM] 		__attribute__ ((aligned (4))); // and USER
unsigned char ATOSE_stack::undefined_stack[STACK_SIZE_UNDEFINED] 	__attribute__ ((aligned (4)));

/*
	ATOSE_STACK::INIT()
	-------------------
	Go through each CPU mode and set the top of stack to a valid address hat we can use.
*/
void ATOSE_stack::init(void)
{
uint32_t status_register;

/*
	Get the current CPU mode
*/
asm volatile
	(
	"mrs %[status_register], cpsr \n"
	: [status_register]"=r"(status_register)
	:
	:
	);

/*
	Set each of the CPU stacks used by the kernel. This does not include
	the user mode stack bacause that is different for each process

	The ARM CPU modes are:
	0b10000 User
	0b10001 FIQ
	0b10010 IRQ
	0b10011 Supervisor
	0b10111 Abort
	0b11011 Undefined
	0b11111 System				(system mode and user mode share the stack)
*/
set_stack(FIRQ_stack + STACK_SIZE_FIRQ, (status_register & ~0x1F) | 0x11);					// FIRQ mode
set_stack(IRQ_stack + STACK_SIZE_IRQ, (status_register & ~0x1F) | 0x12);					// IRQ mode
//set_stack(supervisor_stack + STACK_SIZE_SUPERVISOR, (status_register & ~0x1F) | 0x13);		// supervisor mode (we're in supervisor mode already!)
set_stack(abort_stack + STACK_SIZE_ABORT, (status_register & ~0x1F) | 0x17);				// abort mode
set_stack(undefined_stack + STACK_SIZE_UNDEFINED, (status_register & ~0x1F) | 0x1B);		// undefined mode
set_stack(system_stack + STACK_SIZE_SYSTEM, (status_register & ~0x1F) | 0x1F);				// system (user) mode

/*
	Go back into what ever mode we were in before (Supervisor mode)
*/																
asm volatile
	(
	"msr cpsr, %[status_register] \n"
	:
	: [status_register]"r"(status_register)
	:
	);
}

/*
	ATOSE_STACK::SET_STACK()
	------------------------
	Given the CPU mode (and all the other correctly set PSR buts) go
	into the appropriate mode and set the top of stack to the given value.
*/
void ATOSE_stack::set_stack(void *address, uint32_t mode)
{
/*
	get the current mode
*/
asm volatile
	(
	"mov r0, %[address];"
	"mov r1, %[mode];"
	"mrs r2, cpsr;"
	"msr cpsr, r1;"
	"mov sp, r0;"
	"msr cpsr, r2;"
	:
	: [address]"r"(address), [mode]"r"(mode)
	: "r0", "r1", "r2"
	);
}
