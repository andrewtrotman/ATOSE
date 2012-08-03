/*
	TIMER_IMX233.H
	--------------
*/
#ifndef TIMER_IMX233_H_
#define TIMER_IMX233_H_

#include "timer.h"

class ATOSE_timer_imx233 : public ATOSE_timer
{
public:
	ATOSE_timer_imx233() : ATOSE_timer() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);

	static void delay_us(uint32_t ticks);
} ;



#endif /* TIMER_IMX233_H_ */
