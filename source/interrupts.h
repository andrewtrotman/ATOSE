/*
	INTERRUPTS.H
	------------
*/
#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

extern "C"
{
	void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq(void);
	void __attribute__ ((interrupt("UNDEF"))) __cs3_isr_undef(void);
	void __attribute__ ((interrupt("SWI"))) __cs3_isr_swi(void);
	void __attribute__ ((interrupt("ABORT"))) __cs3_isr_pabort(void);
	void __attribute__ ((interrupt("ABORT"))) __cs3_isr_dabort(void);
	void __attribute__ ((interrupt)) __cs3_isr_reserved(void);
	void __attribute__ ((interrupt("FIQ"))) __cs3_isr_fiq(void);
}



#endif /* INTERRUPTS_H_ */
