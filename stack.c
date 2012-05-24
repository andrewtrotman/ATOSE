/*
	STACK.C
	-------
*/
#include <stdint.h>
#include "stack.h"

unsigned char ATOSE_stack::supervisor_stack[STACK_SIZE_SUPERVISOR];
unsigned char ATOSE_stack::abort_stack[STACK_SIZE_ABORT];
unsigned char ATOSE_stack::undefined_stack[STACK_SIZE_UNDEFINED];
unsigned char ATOSE_stack::IRQ_stack[STACK_SIZE_IRQ];
unsigned char ATOSE_stack::FIRQ_stack[STACK_SIZE_IRQ];

/*
	ATOSE_STACK::ATOSE_STACK()
	--------------------------
*/
ATOSE_stack::ATOSE_stack()
{
uint32_t status_register;
uint32_t top_of_stack = (uint32_t)IRQ_stack + STACK_SIZE_IRQ;
/*
	Get the current mode in r0
	asm(code : output operand list : input operand list : clobber list);
*/
asm volatile
	(
	"mrs %[status_register], cpsr \n"
	: [status_register]"=r"(status_register)
	:
	: "cc"
	);

asm volatile
	(
	/*
		Switch the CPU into IRQ mode
	*/
	"bic r1, %[status_register], #0x1F \n"			/* get the mode bits */
	"orr r1, r1, #0x12 \n"							/* turn on IRQ mode bits */
	"msr cpsr, r1 \n"								/* go into IRQ mode */
	/*																	
		Set the stack pointer											
	*/																	
	"mov sp, %[top_of_stack] \n"
	:
	: [top_of_stack]"r"(top_of_stack), [status_register]"r"(status_register)
	: "r1", "cc"
	);


asm volatile
	(
	/*
		Go back into what ever mode we were in before (Supervisor mode)
	*/																
	"msr cpsr, %[status_register] \n"
	:
	: [status_register]"r"(status_register)
	: "cc"
	);

}