/*
	CPU_ARM_IMX6Q.H
	----------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Special management for the i.MX6Q - specifically manage the CPU interrupt chaining.
*/
#include <stdint.h>

#ifndef CPU_ARM_IMX6Q_H_
#define CPU_ARM_IMX6Q_H_

#include "cpu_arm.h"

/*
	class ATOSE_CPU_ARM_IMX6Q
	-------------------------
*/
class ATOSE_cpu_arm_imx6q : public ATOSE_cpu_arm
{
public:
	ATOSE_cpu_arm_imx6q() : ATOSE_cpu_arm() {}
	virtual void set_irq_handler(void *address);
} ;

#endif
