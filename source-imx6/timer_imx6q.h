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
#include "timer.h"

/*
	class ATOSE_TIMER_IMX6Q
	-----------------------
*/
class ATOSE_timer_imx6q : public ATOSE_timer
{
private:
	static const uint32_t TIME_SLICE_IN_MILLISECONDS = 20;		// 20ms per time slice (i.e. between forced context switches)

public:
	ATOSE_timer_imx6q() : ATOSE_timer() {}
	virtual void initialise(void);

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(ATOSE_registers *registers);
	virtual uint32_t get_interrup_id(void);
} ;

#endif
