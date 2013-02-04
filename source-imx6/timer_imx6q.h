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

#inlcude "device_driver.h"

/*
	class ATOSE_TIMER_IMX6Q
	-----------------------
*/
class ATOSE_timer_imx6q : ATOSE_device_driver
{
public:
	ATOSE_timer_imx6q();
	virtual void init(void);

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;

#endif
