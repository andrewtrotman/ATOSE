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

/*
	class ATOSE_PIPE
	----------------
*/
class ATOSE_pipe
{
private:
	static const uint32_t MAX_PIPES = 16;
	static const uint32_t BUFFER_SIZE = 16;

public:
	enum {SUCCESS, ERROR_PIPE_IN_USE, ERROR_PIPE_NOT_OWNER, ERROR_PIPE_BAD_ID, ERROR_PIPE_OVERFLOW};

private:
	static ATOSE_pipe *pipelist[];
	ATOSE_semaphore semaphore;
	ATOSE_pipe *other_end;
	uint32_t owner;

	/*
		Only one message at a time can be waiting.  All other processes are blocked.
	*/
	volatile char buffer[BUFFER_SIZE];
	volatile uint32_t buffer_length;

public:
	ATOSE_pipe() {}

	uint32_t bind(uint32_t pipe_id);
	uint32_t connect(uint32_t pipe_id);
	uint32_t close(void);
	uint32_t send(void *data, uint32_t length);
	uint32_t receive(void *data, uint32_t length);
	uint32_t peek(void);
};

#endif
