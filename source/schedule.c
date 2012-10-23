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
if (active_head != 0)
	active_head->next = process;	// the queue wasn't empty

process->next = 0;

active_head = process;

if (active_tail == 0)
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

if (active_tail == 0)
	return 0;						// the queue is already empty

answer = active_tail;

if ((active_tail = active_tail->next) == 0)
	active_head = 0;				// the queue is now empty

return answer;
}


