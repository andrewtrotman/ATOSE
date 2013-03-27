/*
	SLEEP_WAKE.C
	------------
*/
#include "atose.h"
#include "sleep_wake.h"

/*
	ATOSE_SLEEP_WAKE::ATOSE_SLEEP_WAKE()
	------------------------------------
*/
ATOSE_sleep_wake::ATOSE_sleep_wake()
{
top = NULL;
}

/*
	ATOSE_SLEEP_WAKE::SLEEP_CURRENT_PROCESS()
	-----------------------------------------
*/
uint32_t ATOSE_sleep_wake::sleep_current_process(void)
{
ATOSE_process *process;

//ATOSE_atose::get_ATOSE()->debug << "[SLEEP:" << (uint32_t)ATOSE_atose::get_ATOSE()->scheduler.get_current_process();

process = ATOSE_atose::get_ATOSE()->scheduler.get_current_process();
ATOSE_atose::get_ATOSE()->scheduler.set_current_process(NULL);

process->next = top;
top = process;

//ATOSE_atose::get_ATOSE()->debug << "]";

return (uint32_t)process;
}

/*
	ATOSE_SLEEP_WAKE::WAKE()
	------------------------
*/
ATOSE_process *ATOSE_sleep_wake::wake(uint32_t id)
{
ATOSE_process **previous, *current, *wanted;

//ATOSE_atose::get_ATOSE()->debug << "[WAKE:" << id;
previous = &top;
wanted = (ATOSE_process *)id;

for (current = top; current != NULL; current = current->next)
	{
	if (current == wanted)
		{
		*previous = current->next;

//		ATOSE_atose::get_ATOSE()->debug << ".";

		ATOSE_atose::get_ATOSE()->scheduler.push(wanted);
		}
	previous = &current->next;
	}

//ATOSE_atose::get_ATOSE()->debug << "]";

return wanted;
}
