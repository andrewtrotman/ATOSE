/*
	SEMAPHORE.C
	-----------
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
void ATOSE_semaphore::sleep_one(ATOSE_registers *registers)
{
ATOSE_atose *os;

os = ATOSE_atose::get_ATOSE();

/*
	pull the current process from the process queue
*/
os->scheduler.get_current_process()->next = sleepers;
sleepers = os->scheduler.get_current_process();
os->scheduler.set_current_process(NULL);

/*
	move on to the next process
*/
os->scheduler.context_switch(registers);
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
void ATOSE_semaphore::wait(ATOSE_registers *registers)
{
value--;

if (value < 0)
	sleep_one(registers);
}

/*
	ATOSE_SEMAPHORE::SIGNAL()
	-------------------------
*/
void ATOSE_semaphore::signal(void)
{
value++;

if (value >= 0)
	wake_one();
}

