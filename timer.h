/*
	TIMER.H
	-------
*/
#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include "device_driver.h"

/*
	class ATOSE_TIMER
	-----------------
*/
class ATOSE_timer : public ATOSE_device_driver
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
	ATOSE_timer();
	virtual ~ATOSE_timer();
	void enable(void);
	void disable(void);
	void acknowledge(void) { *timer_0_intclr = 0; }
} ;

#endif /* TIMER_H_ */
