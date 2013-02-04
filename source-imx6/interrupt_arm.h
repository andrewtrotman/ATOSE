/*
	INTERRUPT_ARM.H
	---------------
*/
#ifndef INTERRUPT_ARM_H_
#define INTERRUPT_ARM_H_

#include <stdint.h>
/*
	class ATOSE_INTERRUPT_ARM
	-------------------------
*/
class ATOSE_interrupt_arm
{
public:
	void *reset;
	void *undefined_instruction;
	void *swi;
	void *prefetch_abort;
	void *data_abort;
	void *reserved;
	void *irq;
	void *firq;
	void *sw_monitor;
} ;

#endif

