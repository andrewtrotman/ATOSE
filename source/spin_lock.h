/*
	SPIN_LOCK.H
	-----------
*/
#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_

#include "lock.h"

/*
	class ATOSE_SPIN_LOCK
	---------------------
*/
class ATOSE_spin_lock : public ATOSE_lock
{
private:
	volatile uint32_t lock;

public:
	ATOSE_spin_lock() { clear(); }

	virtual ATOSE_lock *clear(void) { lock = 0; return this; }
	virtual ATOSE_lock *signal(void) { lock++; return this; }
	virtual ATOSE_lock *wait(void)   { while (lock == 0) /* nothing */; return this; }
} ;

#endif /* SPIN_LOCK_H_ */
