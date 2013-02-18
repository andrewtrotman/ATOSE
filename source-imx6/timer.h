/*
	TIMER.H
	-------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
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
} ;

#endif /* TIMER_H_ */
