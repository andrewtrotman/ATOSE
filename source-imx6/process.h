/*
	PROCESS.H
	---------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdint.h>
#include "registers.h"

class ATOSE_address_space;
class ATOSE_pipe_task;

/*
	class ATOSE_PROCESS
	-------------------
*/
class ATOSE_process
{
public:
	ATOSE_process *next;
	ATOSE_address_space *address_space;
	ATOSE_registers registers;
	ATOSE_pipe_task *current_pipe_task;		// if not NULL then we are blocking waiting for this task
} ;

#endif
