/*
	PIPE_TEST.C
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose_api.h"
#include "atose.h"

static const uint32_t TEST_PIPE = 1;

volatile uint32_t global_pipe = 0;
volatile uint32_t global_client_pipe = 0;

/*
	PROCESS_ONE()
	-------------
*/
uint32_t process_one(void)
{
char buffer[16];
uint32_t message;
uint32_t pipe;

pipe = ATOSE_api::pipe_create();
ATOSE_api::pipe_bind(pipe, TEST_PIPE);
global_pipe = pipe;

while (1)
	{
//	ATOSE_api::write('1');
	message = ATOSE_api::pipe_receive(global_pipe, buffer, 5);
//	ATOSE_api::write('2');
	buffer[0] = '1';
	buffer[5] = '\0';
	ATOSE_api::writeline(buffer);

	buffer[0] = buffer[1] + 1;
//	ATOSE_api::write('3');
	ATOSE_api::pipe_reply(message, buffer, 1);
//	ATOSE_api::write('4');
	}

ATOSE_api::exit(0);
return 0;
}

/*
	PROCESS_TWO()
	-------------
*/
uint32_t process_two(void)
{
char buffer[16];
uint32_t got;

global_client_pipe = ATOSE_api::pipe_create();

do
	got = ATOSE_api::pipe_connect(global_client_pipe, TEST_PIPE);
while (got != 0);

for (uint32_t x = 0; x < 10; x++)
	{
	memset(buffer, 'A' + x, sizeof(buffer));
//	ATOSE_api::write('a');
	ATOSE_api::pipe_send(global_client_pipe, buffer, 5, buffer, 1);
	ATOSE_api::write('&');
	}

ATOSE_api::exit(0);
return 0;
}

/*
	PROCESS_THREE()
	---------------
*/
uint32_t process_three(void)
{
char buffer[16];
uint32_t got;
uint32_t pipe;

pipe = ATOSE_api::pipe_create();

do
	got = ATOSE_api::pipe_connect(pipe, TEST_PIPE);
while (got != 0);

for (uint32_t x = 0; x < 10; x++)
	{
	char become = 'a' + x;

	memset(buffer, become, sizeof(buffer));
//	ATOSE_api::write('a');
	ATOSE_api::pipe_send(pipe, buffer, 5, buffer, 1);
	if (*buffer == become + 1)
		ATOSE_api::write('$');
	else
		ATOSE_api::write('!');
	}

ATOSE_api::exit(0);
return 0;
}

/*
	PROCESS_FOUR()
	--------------
*/
uint32_t process_four(void)
{
char buffer[16];
uint32_t message;

while (global_pipe == 0)
	;	// wait

while (1)
	{
//	ATOSE_api::write('1');
	message = ATOSE_api::pipe_receive(global_pipe, buffer, 5);
//	ATOSE_api::write('2');
	buffer[0] = '4';
	buffer[5] = '\0';
	ATOSE_api::writeline(buffer);

	buffer[0] = buffer[1] + 1;
//	ATOSE_api::write('3');
	ATOSE_api::pipe_reply(message, buffer, 1);
//	ATOSE_api::write('4');
	}

ATOSE_api::exit(0);
return 0;
}


/*
	PIPE_TEST()
	-----------
*/
void pipe_test(void)
{
#ifdef NEVER
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_one);				// server
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_two);				// client
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_three);			// client
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_four);				// server
#endif
ATOSE_atose::get_ATOSE()->scheduler.create_process((uint8_t *)"a.elf");
}
