/*
	TIMER_SP804.H
	-------------
*/
#ifndef TIMER_SP804_H_
#define TIMER_SP804_H_

#include <stdint.h>
#include "timer.h"

/*
	class ATOSE_TIMER_SP804
	-----------------------
*/
class ATOSE_timer_sp804 : public ATOSE_timer
{
private:
	static unsigned char *timer_base_address;

	static volatile uint32_t *timer_0_load;
	static volatile uint32_t *timer_0_value;
	static volatile uint32_t *timer_0_control;
	static volatile uint32_t *timer_0_intclr;
	static volatile uint32_t *timer_0_ris;
	static volatile uint32_t *timer_0_mis;
	static volatile uint32_t *timer_0_bgload;

public:
	ATOSE_timer_sp804() : ATOSE_timer() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;



#endif /* TIMER_SP804_H_ */
