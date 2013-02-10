/*
	TIMER_IMX6Q.H
	-------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This class is the system tick counter used for context switching
*/
#ifndef TIMER_IMX6Q_H_
#define TIMER_IMX6Q_H_

#include <stdint.h>
#include "device_driver.h"

/*
	class ATOSE_TIMER_IMX6Q
	-----------------------
*/
class ATOSE_timer_imx6q : public ATOSE_device_driver
{
public:
	ATOSE_timer_imx6q();

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;

#endif
