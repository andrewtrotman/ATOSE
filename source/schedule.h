/*
	SCHEDULE.H
	----------
*/
#ifndef SCHEDULE_H_
#define SCHEDULE_H_

class ATOSE_process;

/*
	class ATOSE_SCHEDULE
	--------------------
*/
class ATOSE_schedule
{
public:
	ATOSE_process *current_process;

private:
	ATOSE_process *active_head;			// head of the active process list
	ATOSE_process *active_tail;			// tail of the active process list

public:
	ATOSE_schedule() { active_head = active_tail = 0; current_process = 0; }

	void push(ATOSE_process *process);
	ATOSE_process *pull(void);
} ;

#endif /* SCHEDULE_H_ */
