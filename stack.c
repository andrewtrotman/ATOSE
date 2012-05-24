/*
	STACK.C
	-------
*/
#include <stdint.h>
#include "stack.h"

/*
	ATOSE_STACK::ATOSE_STACK()
	--------------------------
*/
ATOSE_stack::ATOSE_stack()
{
uint32_t status_register;
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
	: [top_of_stack]"r"(IRQ_stack_top), [status_register]"r"(status_register)
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