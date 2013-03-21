/*
	THREAD.H
	--------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef THREAD_H_
#define THREAD_H_

#include "registers.h"

class ATOSE_process;

/*
	class ATOSE_THREAD
	------------------
*/
class ATOSE_thread
{
public:
	ATOSE_thread *next;
	ATOSE_process *process;
	ATOSE_registers registers;

public:
	ATOSE_thread() {}
	void initialise(ATOSE_process *process)  { this->process = process; }
} ;

#endif /* THREAD_H_ */
