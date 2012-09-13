/*
	LOCK.H
	------
*/
#ifndef LOCK_H_
#define LOCK_H_

/*
	class ATOSE_LOCK
	----------------
*/
class ATOSE_lock
{
public:
	ATOSE_lock() {}

	virtual ATOSE_lock *clear(void) = 0;
	virtual ATOSE_lock *signal(void) = 0;
	virtual ATOSE_lock *wait(void) = 0;
} ;

#endif /* LOCK_H_ */

