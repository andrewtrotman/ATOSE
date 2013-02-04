/*
	INTERRUPT_ARM_GIC.C
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Management of the general interrupt controller (GIC) discussed in "ARM Generic Interrupt Controller Architecture Specification Version 1"
*/
#include <stdint.h>
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"

/*
	class ATOSE_INTERRUPT_ARM_GIC::ATOSE_INTERRUPT_ARM_GIC()
   --------------------------------------------------------
*/
void ATOSE_interrupt_arm_gic::ATOSE_interrupt_arm_gic(void)
{
uint32_t base;

/*
	Get the address of the CPU's configuration registers, but in the address space of the SOC.
	This instruction only works on the Cortex-A9 MPCore, it does not work on a unicore Cortex A9.
	It has been tested on the i.MX6Q, and as we don't intend to support uni-processors (at the
	moment) this will work for all hardware we're going to support.
*/
asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);

/*
	On the i.MX6Q (is this generally the case for the Cortex A9?) the registers are at
	these fixed locations past the register base.
*/
cpu_registers = (ATOSE_interrupt_arm_gic_cpu *)(base + CPU_REGISTERS_OFFSET_FROM_BASE);
distributor_registers = (ATOSE_interrupt_arm_gic_distributor *)(base + DISTRIBUTOR_REGISTERS_OFFSET_FROM_BASE);

/*
	Enable all interrupt levels (0 = high, 0xFF = low)
*/
cpu_registers->interrupt_priority_mask_register = 0xFF;

/*
   Enable the GIC (both secure and insecure interrupts)
*/
cpu_registers->cpu_interface_control_register = 0x03;					// enable everything
distributor_registers->distributor_control_register = 0x03;			// enable everything
}

/*
	ATOSE_INTERRUPT_ARM_GIC::ENABLE()
	---------------------------------
*/
void ATOSE_interrupt_arm_gic::enable(ATOSE_device_driver *driver, uint32_t source)
{
distributor_registers->interrupt_priority_registers[source] = 0;
distributor_registers->interrupt_security_registers[source / 32] &= ~(1 << (source & 0x1F));
distributor_registers->interrupt_processor_targets_registers[source] |= 1;
distributor_registers->interrupt_set_enable_registers[source / 32] = 1 << (source & 0x1F);

device[source] = driver;
}


