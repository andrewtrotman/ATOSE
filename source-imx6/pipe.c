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
uint32_t ATOSE_pipe::initialise(void)
{
ATOSE_atose::get_ATOSE()->debug.hex();
other_end = NULL;
send_queue = receive_queue = tail_of_receive_queue = 	tail_of_send_queue = NULL;

process = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();
next = process->open_pipes;
process->open_pipes = this;

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
ATOSE_process *owner;
ATOSE_pipe *current, **previous;

/*
	Get the pipe owner and make sure it matches the current process ID
*/
owner = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();
if (owner != process)
	return ERROR_PIPE_NOT_OWNER;

/*
	Kill any active messages
*/
if (owner->current_pipe_task != NULL)
	{
	owner->current_pipe_task->dead = true;
	owner->current_pipe_task = NULL;
	}

/*
	Deliver an event to the other end of the pipe
*/
other_end->post_event(EVENT_CLOSE);

/*
	Walk the list of open pipes and when we find this one remove it.
*/
previous = &owner->open_pipes;
for (current = owner->open_pipes; current != NULL; current = current->next)
	{
	if (current == this)
		{
		*previous = this->next;
		break;
		}
	previous = &current->next;
	}

/*
	Mark the pipe as unowned
*/
process = NULL;

return SUCCESS;
}

/*
	ATOSE_PIPE::MEMCPY()
	--------------------
*/
void ATOSE_pipe::memcpy(ATOSE_process *target_space, void *target_address, ATOSE_process *source_space, void *source_address, uint32_t length)
{
uint8_t buffer[16 * 1024];
uint8_t *from = (uint8_t *)source_address;
uint8_t *into = (uint8_t *)target_address;
ATOSE_mmu *mmu = &ATOSE_atose::get_ATOSE()->heap;

if (source_space->address_space != target_space->address_space)
	{
	/*
		Until we write something cleaner we'll just copy through a buffer guaranteed to be in both spaces (i.e. the kernel stack)
	*/
	while (length > sizeof(buffer))
		{
		mmu->assume(source_space->address_space);
		::memcpy(buffer, from, sizeof(buffer));
		mmu->assume(target_space->address_space);
		::memcpy(into, buffer, sizeof(buffer));

		from += sizeof(buffer);
		into += sizeof(buffer);
		length -= sizeof(buffer);
		}

	mmu->assume(source_space->address_space);
	::memcpy(buffer, from, length);
	mmu->assume(target_space->address_space);
	::memcpy(into, buffer, length);
	}
else
	{
	/*
		We're in the same address space so we can just copy
	*/
	::memcpy(target_address, source_address, length);
	}
}

/*
	ATOSE_PIPE::VALIDATE()
	----------------------
*/
uint32_t ATOSE_pipe::validate(void)
{
return SUCCESS;
/*
if (ATOSE_atose::get_ATOSE()->scheduler.get_current_process() != process)
	return ERROR_PIPE_NOT_OWNER;
*/
}

/*
	ATOSE_PIPE::TASK_ALLOC()
	------------------------
*/
ATOSE_pipe_task *ATOSE_pipe::task_alloc(void)
{
return ATOSE_atose::get_ATOSE()->process_allocator.malloc_pipe_task();
}

/*
	ATOSE_PIPE::FREE()
	------------------
*/
void ATOSE_pipe::free(ATOSE_pipe_task *task)
{
task->dead = true;
task->server = NULL;
task->process->current_pipe_task = NULL;
ATOSE_atose::get_ATOSE()->process_allocator.free(task);
}

/*
	ATOSE_PIPE::ENQUEUE()
	---------------------
*/
void ATOSE_pipe::enqueue(ATOSE_pipe_task *task)
{
ATOSE_pipe_task *into;

/*
	Now check to see if anyone is blocking waiting for us.  If they are we wake them up and tell them to start working again.
*/
if (receive_queue != NULL)
	{
	/*
		Get the item at the head of the queue
	*/
	into = receive_queue;
	if ((receive_queue = receive_queue->next) == NULL)
		tail_of_receive_queue = NULL;

	/*
		Copy the message and return the message ID to the message to the server
	*/
	memcpy(into->process, into->destination, task->process, task->source, min(task->source_length, into->destination_length));
	into->process->execution_path->registers.r0 = (uint32_t)task->event_id;

	/*
		Wake the server
	*/
	ATOSE_atose::get_ATOSE()->scheduler.wake((uint32_t)into->process);
	}
else
	{
	/*
		Else we've got work to do and must wait for a receiver to get ready to receive
	*/
	task->next = NULL;
	if (tail_of_send_queue != NULL)
		tail_of_send_queue->next = task;
	tail_of_send_queue = task;
	if (send_queue == NULL)
		send_queue = task;
	}
}

/*
	ATOSE_PIPE::SEND()
	------------------
	This is the client side
*/
uint32_t ATOSE_pipe::send(void *message, uint32_t length, void *reply, uint32_t reply_length, uint32_t event_id)
{
ATOSE_pipe_task *task;
uint32_t error;

if ((error = validate()) != SUCCESS)
	return error;

/*
	Get a message object
*/
task = task_alloc();
if (task == NULL)
	ATOSE_atose::get_ATOSE()->debug << "[NULL TASK]";

/*
	If we're an event we fake a message
*/
if (message == NULL)
	{
	/*
		We're an event
	*/
	task->source = &task->event_id;
	task->source_length = sizeof(task->event_id);
	task->destination = reply;
	task->destination_length = reply_length;
	task->client = other_end;			// we don't know the client will be alive, but we do know that event is in kernel space so it's also in target space
	task->server = other_end;
	task->process = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();
	task->process->current_pipe_task = task;
	task->dead = false;
	task->event_id = event_id;
	}
else
	{
	task->source = message;
	task->source_length = length;
	task->destination = reply;
	task->destination_length = reply_length;
	task->client = this;
	task->server = other_end;
	task->process = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();
	task->process->current_pipe_task = task;
	task->dead = false;
	task->event_id = (uint32_t)task;
	}

/*
	Place it on the other end's queue and wake it up (if necessary)
*/
other_end->enqueue(task);

/*
	Tell this process to wait for the reply
	reply will be NULL in the case of an event, in which case we don't wait
*/
if (reply != NULL)
	ATOSE_atose::get_ATOSE()->scheduler.sleep_current_process();

return 0;
}

/*
	ATOSE_PIPE::RECEIVE()
	---------------------
	This is the server side
*/
uint32_t ATOSE_pipe::receive(void *data, uint32_t length)
{
ATOSE_pipe_task *task;
uint32_t error;

if ((error = validate()) != SUCCESS)
	return error;

/*
	The event queue might contain a number of tasks that are now "dead" because the client has died.
	To account for this we walk the list until we find the first "live" task, discarding "dead" tasks along the way
*/
while (1)
	{
	/*
		There are two cases here. Either there's a list of stuff to do, or we queue up for it
	*/
	if (send_queue == NULL)
		{
		/*
			We queue up for work that will be copied into our buffers by enqueue()
		*/
		task = task_alloc();
		task->destination = data;
		task->destination_length = length;
		task->process = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();

		/*
			Put it in the queue
		*/
		task->next = NULL;
		if (tail_of_receive_queue != NULL)
			tail_of_receive_queue->next = task;
		tail_of_receive_queue = task;
		if (receive_queue == NULL)
			receive_queue = task;

		/*
			Sleep the server until we get a signal
		*/
		ATOSE_atose::get_ATOSE()->scheduler.sleep_current_process();

		return 0;
		}
	else
		{
		/*
			There's stuff to do so we do the copy and keep going
		*/
		task = send_queue;
		if ((send_queue = send_queue->next) == NULL)
			tail_of_send_queue = NULL;

		if (!task->dead)
			{
			memcpy(ATOSE_atose::get_ATOSE()->scheduler.get_current_process(), data, task->process, task->source, min(length, task->source_length));
			ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->execution_path->registers.r0 = (uint32_t)task;

			return 0;
			}
		}
	}

return 0;
}

/*
	ATOSE_PIPE::REPLY()
	-------------------
*/
uint32_t ATOSE_pipe::reply(uint32_t message_id, void *data, uint32_t length)
{
ATOSE_pipe_task *task = (ATOSE_pipe_task *)message_id;
uint32_t error;

/*
	Validate that we own the pipe
	We should also here make sure the task's address is in the pool.
		the consequence of a task check will be that a call with 0 will correctly fail
*/
if ((error = validate()) != SUCCESS)
	return error;

/*
	Make sure the other end is still living, and if not then we're done.
*/
if (task->dead)
	return 0;

/*
	Copy from the server address space into the client address space
*/
memcpy(task->process, task->destination, ATOSE_atose::get_ATOSE()->scheduler.get_current_process(), data, min(length, task->destination_length));

/*
	Tell the client we're done.  Recall that the other end *must* be blocking waiting for this reply.
*/
ATOSE_atose::get_ATOSE()->scheduler.wake((uint32_t)task->process);

/*
	Return the task back to the task pool
*/
free(task);

return 0;
}
