/*
	INTERRUPT_ARM_GIC.C
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Management of the general interrupt controller (GIC) discussed in "ARM Generic Interrupt Controller Architecture Specification Version 1"
*/
#include <stdint.h>
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"

#include "atose.h"
#include "ascii_str.h"
#include "registers.h"
#include "interrupt_arm_gic.h"
#include "interrupt_arm_gic_cpu.h"
#include "interrupt_arm_gic_distributor.h"

/*
	class ATOSE_INTERRUPT_ARM_GIC::ATOSE_INTERRUPT_ARM_GIC()
   --------------------------------------------------------
*/
ATOSE_interrupt_arm_gic::ATOSE_interrupt_arm_gic() : ATOSE_interrupt()
{
uint32_t base;

/*
	Clear the list of pointers to device drivers
*/
memset(device, 0, sizeof(device));

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

/*
	ATOSE_INTERRUPT_ARM_GIC::ISR_IRQ()
	----------------------------------
*/
void ATOSE_interrupt_arm_gic::isr_irq(ATOSE_registers *registers)
{
uint32_t base;
ATOSE_interrupt_arm_gic_cpu *gic_cpu_registers;
uint32_t got;

/*
	Get the address of the CPU's configuration registers, but in the address space of the SOC.
	This instruction only works on the Cortex-A9 MPCore, it does not work on a unicore Cortex A9.
	It has been tested on the i.MX6Q
*/
asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);
gic_cpu_registers = (ATOSE_interrupt_arm_gic_cpu *)(base + 0x100);

/*
   ACK the interrupt and tell the hardware that we're in the interrupt service routine
*/
got = gic_cpu_registers->interrupt_acknowledge_register;

/*
   Make sure it wasn't a spurious interrupt
	The Cortex A9 MPCore Reference Manual (page 3-3) makes it clear that no semaphore is
	required to avoid the race condition.  This test for spurious is enough
*/
if (got == IMX_INT_SPURIOUS)
   return;

/*
	Save the pointer to the current registers because some device driver might want to access them.
*/
ATOSE_atose::get_ATOSE()->registers_of_interrupted_process = registers;

/*
	Dispatch to the device driver
*/
if (device[got] != NULL)
	device[got]->acknowledge(registers);

/*
	Tell the interrupt controller that we've finished processing the Interrupt
*/
gic_cpu_registers->end_of_interrupt_register = got;

return;
}


