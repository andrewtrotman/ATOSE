/*
	STACK.H
	-------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>

/*
	class ATOSE_STACK
	-----------------
	This object provides a container to store all the different stacks seen on the ARM architecture
*/
class ATOSE_stack
{
private:
	/*
		The ARM CPU has one stack for each mode,  Below is the amount of space we'll allocate for each of them
		The value is specified in bytes.  Actaully, as they must be word alligned (or else you get a CPU fault)
		these numbers are rounded up to the nearest 4-byte amount.
	*/
	static const int STACK_SIZE_FIRQ = 4;
	static const int STACK_SIZE_IRQ = 1024;
	static const int STACK_SIZE_SUPERVISOR = 128;				// used for SWI instructions
	static const int STACK_SIZE_ABORT = 4;
	static const int STACK_SIZE_UNDEFINED = 4;
	static const int STACK_SIZE_SYSTEM = 1024;					// and user

private:
	static uint32_t firq_stack[(STACK_SIZE_FIRQ + 1) / sizeof(uint32_t)];
	static uint32_t irq_stack[(STACK_SIZE_IRQ + 1) / sizeof(uint32_t)];
	static uint32_t supervisor_stack[(STACK_SIZE_SUPERVISOR + 1) / sizeof(uint32_t)];	// SWI stack
	static uint32_t abort_stack[(STACK_SIZE_ABORT + 1) / sizeof(uint32_t)];
	static uint32_t undefined_stack[(STACK_SIZE_UNDEFINED + 1) / sizeof(uint32_t)];
	static uint32_t system_stack[(STACK_SIZE_SYSTEM + 1) / sizeof(uint32_t)];			// and user

private:
	static void set_stack(void *address, uint32_t mode);

public:
	ATOSE_stack();
} ;

#endif /* STACK_H_ */



