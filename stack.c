/*
	STACK.C
	-------
*/
#include "stack.h"

/*
	ATOSE_STACK::ATOSE_STACK()
	--------------------------
*/
ATOSE_stack::ATOSE_stack()
{
/*
	Get the current mode in r0
	asm(code : output operand list : input operand list : clobber list);
*/
asm volatile
	(
	"mrs r0, cpsr \n"
	/*
		Switch the CPU into IRQ mode
	*/
	"bic r1, r0, #0x1F \n"			/* get the mode bits */
	"orr r1, r1, #0x12 \n"			/* turn on IRQ mode bits */
	"msr cpsr, r1 \n"				/* go into IRQ mode */
	/*																	
		Set the stack pointer											
	*/																	
	"mov sp, %[top_of_stack] \n"
	/*
		Go back into what ever mode we were in before (Supervisor mode)
	*/
	"msr cpsr, r0 \n"
	:
	: [top_of_stack]"r"(top_of_stack)
	: "r1", "r2", "cc"
	);

}