/*
	TIMER.H
	-------
*/
#ifndef TIMER_H_
#define TIMER_H_

#include "device_driver.h"

/*
	class ATOSE_TIMER
	-----------------
*/
class ATOSE_timer : public ATOSE_device_driver
{
public:
	ATOSE_timer() : ATOSE_device_driver() {}

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;
	virtual void acknowledge(void) = 0;
} ;

#endif /* TIMER_H_ */
