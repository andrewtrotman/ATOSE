/*
	PIPE.C
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "pipe.h"
#include "pipe_task.h"
#include "ascii_str.h"

template <class T> T min(T a, T b) { return a < b ? a : b; }

/*
	The list of active pipes
*/
ATOSE_pipe *ATOSE_pipe::pipelist[MAX_PIPES] = {0};

/*
	ATOSE_PIPE::INITIALISE()
	------------------------
*/
uint32_t ATOSE_pipe::initialise(ATOSE_process *process)
{
semaphore.clear();
other_end = NULL;
send_queue = receive_queue = NULL;
this->process = process;

return 0;
}

/*
	ATOSE_PIPE::BIND()
	------------------
	returns SUCCESS on success
*/
uint32_t ATOSE_pipe::bind(uint32_t pipe_id)
{
if (pipe_id > MAX_PIPES)
	return ERROR_PIPE_BAD_ID;

if (pipelist[pipe_id] != NULL)
	return ERROR_PIPE_IN_USE;

pipelist[pipe_id] = this;

return SUCCESS;
}

/*
	ATOSE_PIPE::CONNECT()
	---------------------
*/
uint32_t ATOSE_pipe::connect(uint32_t pipe_id)
{
if (pipelist[pipe_id] == NULL)
	return ERROR_PIPE_BAD_ID;

other_end = pipelist[pipe_id];

return SUCCESS;
}

/*
	ATOSE_PIPE::CLOSE()
	-------------------
*/
uint32_t ATOSE_pipe::close()
{
return SUCCESS;
}

/*
	ATOSE_PIPE::ENQUEUE()
	---------------------
*/
void ATOSE_pipe::enqueue(ATOSE_pipe_task *task)
{
ATOSE_pipe_task *into;

/*
	Always bung this task on the queue
*/
task->next = send_queue;
send_queue = task;

/*
	Now check to see if anyone is blocking waiting for us.  If they are we wake them up and tell them to start working again.
	Note that send_queue cannot be NULL because we just added a task to it.
*/
if (receive_queue != NULL)
	{
	into = receive_queue;
	receive_queue = receive_queue->next;
	memcpy(into->destination, send_queue->source, min(send_queue->source_length, into->destination_length));

	/*
		Pass the ID of the task to the process and wake it up
	*/
	process->execution_path->registers.r0 = (uint32_t)task;
	semaphore.signal();
	}
}

/*
	ATOSE_PIPE::DEQUEUE()
	---------------------
*/
ATOSE_pipe_task *ATOSE_pipe::dequeue(void *data, uint32_t length)
{
ATOSE_pipe_task *task;

/*
	There are two cases here. Either there's a list of stuff to do, or we queue up for it
*/
if (send_queue == NULL)
	{
	/*
		We queue up for work that will be copied into our buffers by enqueue()
	*/
	task = ATOSE_atose::get_ATOSE()->process_allocator.malloc_pipe_task();
	task->destination = data;
	task->destination_length = length;

	/*
		Put it in the queue
	*/
	task->next = receive_queue;
	receive_queue = task;

	/*
		Sleep the process until we get a signal
	*/
	semaphore.wait();
	}
else
	{
	/*
		There's stuff to do so we do the copy and keep going
	*/
	task = send_queue;
	send_queue = send_queue->next;

	memcpy(data, task->source, min(length, task->source_length));
	process->execution_path->registers.r0 = (uint32_t)task;
	}

return 0;
}

/*
	ATOSE_PIPE::SEND()
	------------------
	This is the client side
*/
uint32_t ATOSE_pipe::send(void *message, uint32_t length, void *reply, uint32_t reply_length)
{
ATOSE_pipe_task *task;

/*
	Get a message object
*/
task = ATOSE_atose::get_ATOSE()->process_allocator.malloc_pipe_task();

/*
	Fill it
*/
task->source = message;
task->source_length = length;
task->destination = reply;
task->destination_length = reply_length;
task->client = this;
task->server = other_end;

/*
	Place it on the other end's queue and wake it up (if necessary)
*/
other_end->enqueue(task);

/*
	Tell this process to wait for the reply.
*/
semaphore.wait();

return 0;
}

/*
	ATOSE_PIPE::RECEIVE()
	---------------------
	This is the server side
*/
uint32_t ATOSE_pipe::receive(void *data, uint32_t length)
{
/*
	Get the next message from the queue or sleep
*/
return (uint32_t)dequeue(data, length);
}

/*
	ATOSE_PIPE::REPLY()
	-------------------
*/
uint32_t ATOSE_pipe::reply(uint32_t message_id, void *data, uint32_t length)
{
ATOSE_pipe_task *event = (ATOSE_pipe_task *)message_id;

/*
	Copy from the server address space into the client address space
*/
memcpy(event->destination, data, min(length, event->destination_length));

/*
	Tell the client we're done.  Recall that the other end *must* be blocking waiting for this reply.
*/
event->client->semaphore.signal();

/*
	Return the event back to the event pool
*/
ATOSE_atose::get_ATOSE()->process_allocator.free(event);

return 0;
}

