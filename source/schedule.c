/*
	SCHEDULE.C
	----------
*/
#include "process.h"
#include "schedule.h"

/*
	ATOSE_SCHEDULE::PUSH()
	----------------------
	The process list is a queue
*/
void ATOSE_schedule::push(ATOSE_process *process)
{
/*
	Make sure we don't push NULL (so that we can go push(pull())
	when pull() returns NULL,
*/
if (process == NULL)
	return;

if (active_head != NULL)
	active_head->next = process;	// the queue wasn't empty

process->next = NULL;

active_head = process;

if (active_tail == NULL)
	active_tail = process;
}

/*
	ATOSE_SCHEDULE::PULL()
	----------------------
	the process list is a queue
*/
ATOSE_process *ATOSE_schedule::pull(void)
{
ATOSE_process *answer;

if (active_tail == NULL)
	return 0;						// the queue is already empty

answer = active_tail;

if ((active_tail = active_tail->next) == NULL)
	active_head = NULL;				// the queue is now empty

return answer;
}


