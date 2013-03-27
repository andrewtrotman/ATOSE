/*
	SLEEP_WAKE.H
	------------
*/

#ifndef SLEEP_WAKE_H_
#define SLEEP_WAKE_H_

/*
	class ATOSE_SLEEP_WAKE
	----------------------
*/
class ATOSE_sleep_wake
{
private:
	ATOSE_process *top;

public:
	ATOSE_sleep_wake();

	uint32_t sleep_current_process(void);
	ATOSE_process *wake(uint32_t id);
} ;

#endif
