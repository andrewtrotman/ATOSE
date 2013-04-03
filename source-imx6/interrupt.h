/*
	INTERRUPT.H
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <stdint.h>
#include "registers.h"
#include "device_driver.h"

/*
	class ATOSE_INTERRUPT
	---------------------
*/
class ATOSE_interrupt
{
public:
	ATOSE_interrupt() {}
	virtual void initialise(void) {}
	virtual void enable(ATOSE_device_driver *driver, uint32_t source) = 0;

	virtual void isr_irq(ATOSE_registers *registers) = 0;
} ;

#endif
