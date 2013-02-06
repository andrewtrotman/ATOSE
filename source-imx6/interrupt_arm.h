/*
	INTERRUPT_ARM.H
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This structure gives the names and order of the ARM interrupt vectors (which are normally located at address 0x00000000)
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

