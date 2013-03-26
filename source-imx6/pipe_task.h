/*
	PIPE_TASK.H
	-----------
*/
#ifndef PIPE_TASK_H_
#define PIPE_TASK_H_

class ATOSE_pipe;

/*
	class ATOSE_PIPE_TASK
	---------------------
*/
class ATOSE_pipe_task
{
public:
	ATOSE_pipe_task *next;					// these things are chained together
	ATOSE_pipe *client;						// the client's end of the pipe
	ATOSE_pipe *server;						// the server's end of the pipe
	ATOSE_process *process;					// the process that is blocked on this message

	void *source;								// copy from
	uint32_t source_length;
	void *destination;						// copy to
	uint32_t destination_length;
} ;

#endif
