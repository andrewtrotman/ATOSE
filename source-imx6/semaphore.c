/*
	SEMAPHORE.C
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "semaphore.h"

/*
	ATOSE_SEMAPHORE::WAKE_ONE()
	---------------------------
*/
void ATOSE_semaphore::wake_one(void)
{
ATOSE_process *got;

if (sleepers == NULL)
	return;

got = sleepers;
sleepers = sleepers->next;
ATOSE_atose::get_ATOSE()->scheduler.push(got);
}

/*
	ATOSE_SEMAPHORE::SLEEP_ONE()
	----------------------------
*/
void ATOSE_semaphore::sleep_one(void)
{
ATOSE_atose *os;
ATOSE_process *process;

os = ATOSE_atose::get_ATOSE();

/*
	pull the current process from the process queue
*/
process = os->scheduler.get_current_process();
os->scheduler.set_current_process(NULL);

process->next = sleepers;
sleepers = process;
}

/*
	ATOSE_SEMAPHORE::CLEAR()
	------------------------
*/
void ATOSE_semaphore::clear(void)
{
value = 0;
sleepers = NULL;
}

/*
	ATOSE_SEMAPHORE::WAIT()
	-----------------------
*/
void ATOSE_semaphore::wait(void)
{
value--;

if (value < 0)
	sleep_one();
}

/*
	ATOSE_SEMAPHORE::SIGNAL()
	-------------------------
*/
void ATOSE_semaphore::signal(void)
{
value++;

if (value <= 0)
	wake_one();
}
