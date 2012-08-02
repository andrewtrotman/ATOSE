/*
	INTERRUPTS.H
	------------
*/
#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include "registers.h"

extern "C"
{
	int __cxa_pure_virtual(void) __attribute__((noinline, used));						// used by the C++ compiler when a pure virtual function is called.
	void __attribute__ ((interrupt("UNDEF"))) __cs3_isr_undef(void);
	void __attribute__ ((interrupt("ABORT"))) __cs3_isr_pabort(void);
	void __attribute__ ((interrupt("ABORT"))) __cs3_isr_dabort(void);
	void __attribute__ ((interrupt)) __cs3_isr_reserved(void);
	void __attribute__ ((interrupt("FIQ"))) __cs3_isr_fiq(void);

	uint32_t ATOSE_isr_irq(ATOSE_registers *registers);
	uint32_t ATOSE_isr_swi(ATOSE_registers *registers);
}


#endif /* INTERRUPTS_H_ */
