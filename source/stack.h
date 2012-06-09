/*
	STACK.H
	-------
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
	static const int STACK_SIZE_FIRQ = 4096;
	static const int STACK_SIZE_IRQ = 4096;
	static const int STACK_SIZE_SUPERVISOR = 4096;
	static const int STACK_SIZE_ABORT = 4096;
	static const int STACK_SIZE_UNDEFINED = 4096;
	static const int STACK_SIZE_SYSTEM = 4096;					// and user

private:
	static unsigned char FIRQ_stack[STACK_SIZE_FIRQ];
	static unsigned char IRQ_stack[STACK_SIZE_IRQ];
	static unsigned char supervisor_stack[STACK_SIZE_SUPERVISOR];
	static unsigned char abort_stack[STACK_SIZE_ABORT];
	static unsigned char undefined_stack[STACK_SIZE_UNDEFINED];
	static unsigned char system_stack[STACK_SIZE_SYSTEM];		// and user

private:
	void set_stack(void *address, uint32_t mode);

public:
	ATOSE_stack() {}

	void init(void);
} ;

#endif /* STACK_H_ */



