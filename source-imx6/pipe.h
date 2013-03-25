/*
	PIPE.H
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef PIPE_H_
#define PIPE_H_

#include <stdint.h>
#include "semaphore.h"

class ATOSE_pipe_task;

/*
	class ATOSE_PIPE
	----------------
*/
class ATOSE_pipe
{
private:
	static const uint32_t MAX_PIPES = 16;

public:
	enum {SUCCESS, ERROR_PIPE_IN_USE, ERROR_PIPE_NOT_OWNER, ERROR_PIPE_BAD_ID, ERROR_PIPE_OVERFLOW};

private:
	static ATOSE_pipe *pipelist[];
	ATOSE_pipe *other_end;
	ATOSE_pipe_task *send_queue;				// this is send() to this node, not "outgoing"
	ATOSE_pipe_task *receive_queue;			// this is receive() on this node, which is "incoming"
	ATOSE_process *process;
	ATOSE_semaphore semaphore;

private:
	void enqueue(ATOSE_pipe_task *task);
	ATOSE_pipe_task *dequeue(void *data, uint32_t length);

public:
	ATOSE_pipe() {}
	uint32_t initialise(ATOSE_process *process);

	uint32_t bind(uint32_t pipe_id);
	uint32_t connect(uint32_t pipe_id);
	uint32_t close(void);
	uint32_t send(void *message, uint32_t length, void *reply, uint32_t reply_length);
	uint32_t receive(void *data, uint32_t length);
	uint32_t reply(uint32_t message_id, void *data, uint32_t length);
};

#endif
