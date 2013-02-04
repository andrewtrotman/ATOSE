/*
	CPU_ARM_IMX6Q.H
	----------------
*/
#include <stdint.h>

#ifndef CPU_ARM_IMX6Q_H_
#define CPU_ARM_IMX6Q_H_

#include "arm_cpu.h"

/*
	class ATOSE_CPU_ARM_IMX6Q
	-------------------------
*/
class ATOSE_cpu_arm_imx6q : ATOSE_cpu_arm
{
public:
	ATOSE_cpu_arm_imx6q() : cpu_arm() {}
	virtual void set_irq_handler(void *address);
} ;

#endif
