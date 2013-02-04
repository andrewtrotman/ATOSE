/*
	INTERRUPT_ARM_GIC.H
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Management of the general interrupt controller (GIC) discussed in "ARM Generic Interrupt Controller Architecture Specification Version 1"
*/
#ifndef INTERRUPT_ARM_GIC_H_
#define INTERRUPT_ARM_GIC_H_

#include <stdint.h>
#include "device_driver.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"

/*
	class ATOSE_INTERRUPT_ARM_GIC
	-----------------------------
*/
class ATOSE_interrupt_arm_gic
{
private:
	static const uint32_t IMX_INT_SPURIOUS = 1023;									// Due to race conditions the GIC can generate spurious interrupts which it then signals as coming from source 1023
	static const uint32_t CPU_REGISTERS_OFFSET_FROM_BASE = 0x100;				// true for the i.MX6Q, generally for the Cortex A9?
	static const uint32_t DISTRIBUTOR_REGISTERS_OFFSET_FROM_BASE = 0x1000;  // true for the i.MX6Q, generally for the Cortex A9?
	static const uint32_t MAX_INTERRUPT_VECTOR = IMX_INTERRUPT_COUNT;			// true for the i.MX6Q - the general case is 1024 (the i.MX6Q case is 160 (see irq_numbers.h))

private:
	volatile ATOSE_interrupt_arm_gic_distributor *distributor_registers;		// the CPU-specific registers
	volatile ATOSE_interrupt_arm_gic_cpu *cpu_registers;							// the core-specific registers
	ATOSE_device_driver *device[MAX_INTERRUPT_VECTOR];								// for each interrupt, this table points to the device driver object

public:
	ATOSE_interrupt_arm_gic();
	void enable(ATOSE_device_driver *driver, uint32_t source);
} ;

#endif
