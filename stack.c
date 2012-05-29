/*
	STACK.C
	-------
	asm(code : output operand list : input operand list : clobber list);
*/
#include "stack.h"

unsigned char ATOSE_stack::FIRQ_stack[STACK_SIZE_IRQ];
unsigned char ATOSE_stack::IRQ_stack[STACK_SIZE_IRQ];
unsigned char ATOSE_stack::supervisor_stack[STACK_SIZE_SUPERVISOR];
unsigned char ATOSE_stack::abort_stack[STACK_SIZE_ABORT];
unsigned char ATOSE_stack::undefined_stack[STACK_SIZE_UNDEFINED];
unsigned char ATOSE_stack::system_stack[STACK_SIZE_SUPERVISOR];

/*
	ATOSE_STACK::ATOSE_STACK()
	--------------------------
	Go through each CPU mode and set the top of stack to a valid address hat we can use.
*/
ATOSE_stack::ATOSE_stack()
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
set_stack(system_stack + STACK_SIZE_SYSTEM, (status_register & ~0x1F) | 0x1F);				// system mode

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
uint32_t status_register;

/*
	get the current mode
*/
asm volatile
	(
	"mrs %[status_register], cpsr \n"
	: [status_register]"=r"(status_register)
	:
	:
	);

/*
	change mode and set the stack pointer
*/
asm volatile
	(
	"msr cpsr, %[mode] \n"
	"mov sp, %[address] \n"
	:
	: [mode]"r"(mode), [address]"r"((uint32_t)address)
	);

/*
	Go back to what ever mode we were in before
*/
asm volatile
	(
	"msr cpsr, %[status_register] \n"
	:
	: [status_register]"r"(status_register)
	:
	);
}