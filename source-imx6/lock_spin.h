/*
	LOCK_SPIN.H
	-----------
*/
#ifndef LOCK_SPIN_H_
#define LOCK_SPIN_H_

#include "lock.h"

/*
	class ATOSE_LOCK_SPIN
	---------------------
*/
class ATOSE_lock_spin : public ATOSE_lock
{
private:
	volatile uint32_t lock;

public:
	ATOSE_lock_spin() 					{ clear(); }

	virtual ATOSE_lock *clear(void) 	{ lock = 0; return this; }
	virtual ATOSE_lock *signal(void) 	{ lock++; return this; }
	virtual ATOSE_lock *wait(void)   	{ while (lock == 0) /* nothing */; return this; }
	virtual uint32_t get(void) 			{ return lock; }
} ;

#endif /* LOCK_SPIN_H_ */

