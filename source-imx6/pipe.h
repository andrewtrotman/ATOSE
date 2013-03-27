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
class ATOSE_process;

/*
	class ATOSE_PIPE
	----------------
*/
class ATOSE_pipe
{
private:
	static const uint32_t MAX_PIPES = 128;

public:
	enum {SUCCESS, ERROR_PIPE_IN_USE, ERROR_PIPE_NOT_OWNER, ERROR_PIPE_BAD_ID};
	enum {EVENT_CLOSE = 1};

private:
	static ATOSE_pipe *pipelist[];
	ATOSE_pipe *other_end;
	ATOSE_pipe_task *send_queue;						// this is send() to this node, not "outgoing"
	ATOSE_pipe_task *receive_queue;					// this is receive() on this node, which is "incoming"
	ATOSE_pipe_task *tail_of_send_queue;			// this is the insertion end of the queue
	ATOSE_pipe_task *tail_of_receive_queue;		// this is the insertion end of the queue
	ATOSE_process *process;								// the process that owns this end of the pipe

public:
	ATOSE_pipe *next;										// so that we can build lists of these

private:
	void memcpy(ATOSE_process *target_space, void *target_address, ATOSE_process *source_space, void *source_address, uint32_t length);
	void enqueue(ATOSE_pipe_task *task);
	uint32_t validate(void);
	ATOSE_pipe_task *task_alloc(void);
	void free(ATOSE_pipe_task *task);

public:
	ATOSE_pipe() {}
	uint32_t initialise(void);

	uint32_t bind(uint32_t pipe_id);
	uint32_t connect(uint32_t pipe_id);
	uint32_t close(void);
	uint32_t send(void *message, uint32_t length, void *reply, uint32_t reply_length, uint32_t event_id = 0);
	uint32_t post_event(uint32_t event) { return send(0, 0, 0, 0, event); }
	uint32_t receive(void *data, uint32_t length);
	uint32_t reply(uint32_t message_id, void *data, uint32_t length);
};

#endif
