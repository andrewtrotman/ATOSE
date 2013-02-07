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
#include "clock_imx6q.h"

/*
	class ATOSE_CPU_ARM_IMX6Q
	-------------------------
*/
class ATOSE_cpu_arm_imx6q : public ATOSE_cpu_arm
{
private:
	ATOSE_clock_imx6q clock;

public:
	ATOSE_cpu_arm_imx6q() : ATOSE_cpu_arm() {}
	virtual void set_irq_handler(void *address);
	virtual void delay_us(uint32_t time_in_useconds) { clock.delay_us(time_in_useconds); }
} ;

#endif
