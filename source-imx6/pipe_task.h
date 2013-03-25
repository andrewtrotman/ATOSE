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
	ATOSE_pipe_task *next;
	ATOSE_pipe *client;
	ATOSE_pipe *server;

	void *source;
	uint32_t source_length;
	void *destination;
	uint32_t destination_length;
} ;

#endif
