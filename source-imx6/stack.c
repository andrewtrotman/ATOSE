/*
	STACK.C
	-------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "stack.h"
#include "cpu_arm.h"

uint32_t ATOSE_stack::firq_stack[(STACK_SIZE_FIRQ + 1) / sizeof(uint32_t)];
uint32_t ATOSE_stack::irq_stack[(STACK_SIZE_IRQ + 1) / sizeof(uint32_t)];
uint32_t ATOSE_stack::supervisor_stack[(STACK_SIZE_SUPERVISOR + 1) / sizeof(uint32_t)];	// SWI stack
uint32_t ATOSE_stack::abort_stack[(STACK_SIZE_ABORT + 1) / sizeof(uint32_t)];
uint32_t ATOSE_stack::undefined_stack[(STACK_SIZE_UNDEFINED + 1) / sizeof(uint32_t)];
uint32_t ATOSE_stack::system_stack[(STACK_SIZE_SYSTEM + 1) / sizeof(uint32_t)];			// and user

/*
	ATOSE_STACK::SET_STACK()
	------------------------
	Given the CPU mode (and all the other correctly set PSR buts) go
	into the appropriate mode and set the top of stack to the given value.
*/
inline void ATOSE_stack::set_stack(void *address, uint32_t mode)
{
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

/*
	ATOSE_STACK::INITIALISE()
	-------------------------
	Go through each CPU mode and set the top of stack to a valid address hat we can use.
*/
void ATOSE_stack::initialise(void)
{
uint32_t status_register, current_mode;

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
	Set each of the CPU stacks used by the kernel but make sure we don't disrupt the current stack
*/
current_mode = status_register & ATOSE_cpu_arm::MODE_BITS;

if (current_mode != ATOSE_cpu_arm::MODE_FIRQ)
	set_stack(((uint8_t *)firq_stack) + STACK_SIZE_FIRQ, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_FIRQ);
if (current_mode != ATOSE_cpu_arm::MODE_IRQ)
	set_stack(((uint8_t *)irq_stack) + STACK_SIZE_IRQ, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_IRQ);
if (current_mode != ATOSE_cpu_arm::MODE_SUPERVISOR)
	set_stack(((uint8_t *)supervisor_stack) + STACK_SIZE_SUPERVISOR, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_SUPERVISOR);
if (current_mode != ATOSE_cpu_arm::MODE_ABORT)
	set_stack(((uint8_t *)abort_stack) + STACK_SIZE_ABORT, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_ABORT);
if (current_mode != ATOSE_cpu_arm::MODE_UNDEFINED)
	set_stack(((uint8_t *)undefined_stack) + STACK_SIZE_UNDEFINED, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_UNDEFINED);
if (current_mode != ATOSE_cpu_arm::MODE_SYSTEM)
	set_stack(((uint8_t *)system_stack) + STACK_SIZE_SYSTEM, (status_register & ~ATOSE_cpu_arm::MODE_BITS) | ATOSE_cpu_arm::MODE_SYSTEM);				// system and user mode share the stack

/*
	Go back into what ever mode we were in before (probably Supervisor mode)
*/
asm volatile
	(
	"msr cpsr, %[status_register] \n"
	:
	: [status_register]"r"(status_register)
	:
	);
}
