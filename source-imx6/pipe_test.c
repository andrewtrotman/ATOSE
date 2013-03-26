/*
	PIPE_TEST.C
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose_api.h"
#include "atose.h"

static const uint32_t TEST_PIPE = 1;

/*
	PROCESS_ONE()
	-------------
*/
uint32_t process_one(void)
{
char buffer[16];
uint32_t pipe, message;

pipe = ATOSE_api::pipe_create();
ATOSE_api::pipe_bind(pipe, TEST_PIPE);

for (uint32_t x = 0; x < 10; x++)
	{
//	ATOSE_api::write('1');
	message = ATOSE_api::pipe_receive(pipe, buffer, 5);
//	ATOSE_api::write('2');

	buffer[5] = '\0';
	ATOSE_api::writeline(buffer);

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
uint32_t pipe, got;

pipe = ATOSE_api::pipe_create();

do
	got = ATOSE_api::pipe_connect(pipe, TEST_PIPE);
while (got != 0);

for (volatile uint32_t x = 0; x < 5000000; x++);


for (uint32_t x = 0; x < 10; x++)
	{
	memset(buffer, 'A' + x, sizeof(buffer));
//	ATOSE_api::write('a');
	ATOSE_api::pipe_send(pipe, buffer, 5, buffer, 1);
//	ATOSE_api::write('b');
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
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_one);
ATOSE_atose::get_ATOSE()->scheduler.create_system_thread(process_two);
}
