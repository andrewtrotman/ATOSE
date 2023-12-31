/*
	ATOSE_API.H
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef ATOSE_API_H_
#define ATOSE_API_H_

#include <stdint.h>
#include "address_space.h"

/*
	The SWI number used for standard system calls
*/
#define ATOSE_SWI 0x4154

/*
	List of ATOSE operating system entry points
*/
enum
	{
	ATOSE_WRITE_BYTE,
	ATOSE_READ_BYTE,
	ATOSE_PEEK_BYTE,
	ATOSE_SPAWN,
	ATOSE_BEGIN_THREAD,
	ATOSE_YIELD,
	ATOSE_EXIT,
	ATOSE_SET_HEAP_BREAK,
	ATOSE_GET_HEAP_BREAK,
	ATOSE_SEMAPHORE_CREATE,
	ATOSE_SEMAPHORE_CLEAR,
	ATOSE_SEMAPHORE_SIGNAL,
	ATOSE_SEMAPHORE_WAIT,
	ATOSE_PIPE_CREATE,
	ATOSE_PIPE_BIND,
	ATOSE_PIPE_CONNECT,
	ATOSE_PIPE_CLOSE,
	ATOSE_PIPE_SEND,
	ATOSE_PIPE_POST_EVENT,
	ATOSE_PIPE_RECEIVE,
	ATOSE_PIPE_MEMCPY,
	ATOSE_PIPE_REPLY,
	ATOSE_END_OF_METHODS
	} ;

/*
	class ATOSE_API
	---------------
*/
class ATOSE_api
{
protected:
	static uint32_t read_key(void);
	static uint32_t getxy(uint32_t *x, uint32_t *y);
	static uint32_t gotoxy(uint32_t x, uint32_t y);
public:
	ATOSE_api() {}

	static uint32_t SYSTEM_CALL(uint32_t function_id, uint32_t parameter_1 = 0, uint32_t parameter_2 = 0, uint32_t parameter_3 = 0, uint32_t parameter_4 = 0, uint32_t parameter_5 = 0)
	{
	uint32_t answer;

	asm volatile
		(
		"mov r0, %[function_id];"
		"mov r1, %[parameter_1];"
		"mov r2, %[parameter_2];"
		"mov r3, %[parameter_3];"
		"mov r4, %[parameter_4];"
		"mov r5, %[parameter_5];"
		"swi %[ATOSE_swi];"
		"mov %[answer], r0;"
		: [answer]"=r" (answer)
		: [function_id]"r"(function_id),
        [parameter_1]"r"(parameter_1),
        [parameter_2]"r"(parameter_2),
        [parameter_3]"r"(parameter_3),
        [parameter_4]"r"(parameter_4),
        [parameter_5]"r"(parameter_5),
        [ATOSE_swi]"i"(ATOSE_SWI)
		: "r0", "r1", "r2", "r3", "r4", "r5"
		);
	return answer;
	}

	static uint32_t write(uint8_t byte)                                         { return SYSTEM_CALL(ATOSE_WRITE_BYTE, byte); }
	static uint32_t read(void)                                                  { return SYSTEM_CALL(ATOSE_READ_BYTE); }
	static uint32_t peek(void)                                                  { return SYSTEM_CALL(ATOSE_PEEK_BYTE); }
	static uint32_t spawn(const char *elf_filename)                             { return SYSTEM_CALL(ATOSE_SPAWN, (uint32_t)elf_filename); }
	static uint32_t begin_thread(uint32_t (*address)())                         { return SYSTEM_CALL(ATOSE_BEGIN_THREAD, (uint32_t)address); }
	static uint32_t yield(void)                                                 { return SYSTEM_CALL(ATOSE_YIELD); }
	static uint32_t exit(uint32_t return_code)                                  { return SYSTEM_CALL(ATOSE_EXIT, return_code); }
	static uint32_t set_heap_break(uint32_t bytes, uint32_t permissions = ATOSE_address_space::READ | ATOSE_address_space::WRITE)     { return SYSTEM_CALL(ATOSE_SET_HEAP_BREAK, bytes, permissions); }
	static uint8_t *get_heap_break(void)                                        { return (uint8_t *)SYSTEM_CALL(ATOSE_GET_HEAP_BREAK); }

	static uint32_t semaphore_create(void)                                      { return SYSTEM_CALL(ATOSE_SEMAPHORE_CREATE); }
	static uint32_t semaphore_clear(uint32_t handle)                            { return SYSTEM_CALL(ATOSE_SEMAPHORE_CLEAR, handle); }
	static uint32_t semaphore_signal(uint32_t handle)                           { return SYSTEM_CALL(ATOSE_SEMAPHORE_SIGNAL, handle); }
	static uint32_t semaphore_wait(uint32_t handle)                             { return SYSTEM_CALL(ATOSE_SEMAPHORE_WAIT, handle); }

	static char *readline(char *buffer, uint32_t length);
	static char *writeline(const char *string);

	static uint32_t pipe_create(void)                                           { return SYSTEM_CALL(ATOSE_PIPE_CREATE); }
	static uint32_t pipe_bind(uint32_t pipe_id, uint32_t port)                  { return SYSTEM_CALL(ATOSE_PIPE_BIND, pipe_id, port); }
	static uint32_t pipe_connect(uint32_t pipe_id, uint32_t port)               { return SYSTEM_CALL(ATOSE_PIPE_CONNECT, pipe_id, port); }
	static uint32_t pipe_close(uint32_t pipe_id)                                { return SYSTEM_CALL(ATOSE_PIPE_CLOSE, pipe_id); }
	static uint32_t pipe_send(uint32_t pipe_id, volatile void *destination, uint32_t destination_length, volatile void *source, uint32_t source_length) { return SYSTEM_CALL(ATOSE_PIPE_SEND, pipe_id, (uint32_t)destination, destination_length, (uint32_t)source, source_length); }
	static uint32_t pipe_post_event(uint32_t pipe_id, uint32_t event_id)                                                              { return SYSTEM_CALL(ATOSE_PIPE_POST_EVENT, pipe_id, event_id); }
	static uint32_t pipe_receive(uint32_t pipe_id, void *data, uint32_t length, uint32_t *client_process_id = 0)                      { return SYSTEM_CALL(ATOSE_PIPE_RECEIVE, pipe_id, (uint32_t)data, length, (uint32_t)client_process_id); }
	static uint32_t pipe_memcpy(uint32_t message_id, uint32_t destination_offset, void *source, uint32_t length)                      { return SYSTEM_CALL(ATOSE_PIPE_MEMCPY, message_id, destination_offset, (uint32_t)source, length); }
	static uint32_t pipe_reply(uint32_t message_id, void *data, uint32_t length, uint32_t return_code = 0)                            { return SYSTEM_CALL(ATOSE_PIPE_REPLY, message_id, (uint32_t)data, length, return_code); }
} ;

#endif
