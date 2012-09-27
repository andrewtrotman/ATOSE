/*
	LOCK_COUNTDOWN_TRIGGER.H
	------------------------
*/
#ifndef LOCK_COUNTDOWN_TRIGGER_H_
#define LOCK_COUNTDOWN_TRIGGER_H_

#include "lock.h"

/*
	class ATOSE_LOCK_COUNTDOWN_TRIGGER
	----------------------------------
*/
class ATOSE_lock_countdown_trigger : public ATOSE_lock
{
private:
	volatile uint32_t lock;
	volatile uint32_t cap;

public:
	ATOSE_lock_countdown_trigger(uint32_t count_from) 		{ cap = count_from; clear(); }

	virtual ATOSE_lock *clear(void) 	{ lock = cap; return this; }
	virtual ATOSE_lock *signal(void) 	{ lock--; return this; }
	virtual ATOSE_lock *wait(void)   	{ while (lock != 0) /* nothing */; return this; }
	virtual uint32_t get(void) 			{ return lock; }
} ;



#endif /* LOCK_COUNTDOWN_TRIGGER_H_ */
