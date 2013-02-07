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

/*
	class ATOSE_ATOSE
	-----------------
*/
class ATOSE_atose
{
private:
	/*
		A few references for syntactic purposes (i.e. standard methods to refer to these objects regardless of subclassing)
	*/
	ATOSE_debug &debug;
	ATOSE_cpu_arm &cpu;

	/*
		NOTE: In C++ objects are constructed in the order in which they are specified in the class
	*/
	ATOSE_stack stack;
	ATOSE_cpu_arm_imx6q imx6q_cpu;
	ATOSE_uart_imx6q imx6q_serial_port;

public:
	ATOSE_atose();

	void boot(void);
} ;

#endif /* ATOSE_H_ */
