/*
	ATOSE.H
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef ATOSE_H_
#define ATOSE_H_

#include "stack.h"
#include "debug.h"
#include "cpu_arm.h"
#include "uart_imx6q.h"
#include "cpu_arm_imx6q.h"

class ATOSE_registers;

/*
	class ATOSE_ATOSE
	-----------------
*/
class ATOSE_atose
{
private:
	/*
		NOTE: In C++ objects are constructed in the order in which they are specified in the class
	*/
	ATOSE_stack stack;
	ATOSE_cpu_arm_imx6q imx6q_cpu;
	ATOSE_uart_imx6q imx6q_serial_port;

public:
	/*
		A few references for syntactic purposes (i.e. standard methods to refer to these objects regardless of subclassing)
	*/
	ATOSE_debug &debug;
	ATOSE_cpu_arm &cpu;

public:
	ATOSE_atose();

	static ATOSE_atose *get_ATOSE(void) { extern uint32_t ATOSE_pointer; return ((ATOSE_atose *)(&ATOSE_pointer)); }

	virtual void reset(ATOSE_registers *registers = NULL);		// we can't normally have the registers on reset.
	virtual void isr_irq(ATOSE_registers *registers);
	virtual void isr_firq(ATOSE_registers *registers);
	virtual void isr_swi(ATOSE_registers *registers);
	virtual void isr_prefetch_abort(ATOSE_registers *registers);
	virtual void isr_data_abort(ATOSE_registers *registers);
	virtual void isr_undefined(ATOSE_registers *registers);
	virtual void isr_reserved(ATOSE_registers *registers);
} ;

#endif /* ATOSE_H_ */

