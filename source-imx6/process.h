/*
	PROCESS.H
	---------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdint.h>

class ATOSE_address_space;
class ATOSE_thread;
class ATOSE_pipe_task;
class ATOSE_pipe;

/*
	class ATOSE_PROCESS
	-------------------
*/
class ATOSE_process
{
public:
	ATOSE_process *next;
	ATOSE_address_space *address_space;
	ATOSE_thread *execution_path;
//	uint8_t *entry_point;						// initial process entry point
	ATOSE_pipe_task *current_pipe_task;		// if not NULL then we are blocking waiting for this task
	ATOSE_pipe *open_pipes;						// a list of pipes this process currently holds open (as the client)
} ;

#endif
