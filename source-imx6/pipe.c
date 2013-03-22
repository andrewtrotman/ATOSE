/*
	PIPE.C
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "pipe.h"
#include "ascii_str.h"

/*
	The list of active pipes
*/
ATOSE_pipe *ATOSE_pipe::pipelist[MAX_PIPES] = {0};

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
owner = true;
buffer_length = 0;

return SUCCESS;
}

/*
	ATOSE_PIPE::CONNECT()
	---------------------
*/
uint32_t ATOSE_pipe::connect(uint32_t pipe_id)
{
owner = false;
other_end = pipelist[pipe_id];
other_end->other_end = this;

return SUCCESS;
}

/*
	ATOSE_PIPE::CLOSE()
	-------------------
*/
uint32_t ATOSE_pipe::close()
{
if (!owner)
	return ERROR_PIPE_NOT_OWNER;

return SUCCESS;
}

/*
	ATOSE_PIPE::SEND()
	------------------
*/
uint32_t ATOSE_pipe::send(void *data, uint32_t length)
{
if (length > BUFFER_SIZE)
	return ERROR_PIPE_OVERFLOW;

/*
	Wait until the other end says we can write
*/
semaphore.wait();

/*
	Copy the data
*/
memcpy((void *)other_end->buffer, data, length);
other_end->buffer_length = length;

/*
	Tell the other end that we have data for it.
*/
other_end->semaphore.signal();

return SUCCESS;
}

/*
	ATOSE_PIPE::RECEIVE()
	---------------------
*/
uint32_t ATOSE_pipe::receive(void *data, uint32_t length)
{
/*
	Wait until we have data to process
*/
semaphore.wait();

/*
	Copy the data
*/
if (length > buffer_length)
	return ERROR_PIPE_OVERFLOW;
else
	memcpy(data, (void *)buffer, length);

buffer_length = 0;

/*
	We're ready for more data
*/
other_end->semaphore.signal();

return SUCCESS;
}

/*
	ATOSE_PIPE::PEEK()
	------------------
*/
uint32_t ATOSE_pipe::peek(void)
{
return buffer_length;
}
