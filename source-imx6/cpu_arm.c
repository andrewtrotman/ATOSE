/*
	CPU_ARM.C
	---------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>
#include "cpu_arm.h"

/*
	ATOSE_CPU_ARM::GET_CPSR()
	-------------------------
*/
uint32_t ATOSE_cpu_arm::get_cpsr(void)
{
uint32_t answer;

asm volatile
	(
	"mrs %0,CPSR"
	:"=r" (answer)
	);

return answer;
}

/*
	ATOSE_CPU_ARM::SET_CPSR()
	-------------------------
*/
void ATOSE_cpu_arm::set_cpsr(uint32_t save_cpsr)
{
asm volatile
	(
	"msr CPSR_cxsf, %0"
	:
	:"r"(save_cpsr)
	);
}

/*
	ATOSE_CPU_ARM::ENABLE_IRQ()
	---------------------------
*/
void ATOSE_cpu_arm::enable_irq(void)
{
set_cpsr(get_cpsr() & ~0x80);
}

/*
	ATOSE_CPU_ARM::IRQ_HANDLER()
	----------------------------
*/
void ATOSE_cpu_arm::irq_handler(void)
{
static uint32_t irq_stack[irq_stack_size / sizeof(uint32_t)];			// we use a uint32_t so that the stack is correctly aligned for CPU accesses
uint32_t *irq_stack_pointer = irq_stack + sizeof(irq_stack);			// the stack grows down from here.

/*
	Set up the IRQ stack from scratch each time (for cleanliness)
*/
asm volatile
(
	"mov sp, %[top_of_stack];"			// set the stack top
	:
	: [top_of_stack]"r"(irq_stack_pointer)
);
}

