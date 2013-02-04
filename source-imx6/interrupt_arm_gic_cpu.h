/*
	INTERRUPT_ARM_GIC_CPU.H
	-----------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	According to page 2-2 of  "ARM Generic Interrupt Controller Architecture Specification Version 1"

	"This GIC architecture splits logically into a Distributor block and one or more CPU interface blocks, as
	Figure 2-1 on page 2-3 shows:
	Distributor 		This performs interrupt prioritization and distribution to the CPU interfaces that connect to the processors in the system.
	CPU interfaces 	Each CPU interface performs priority masking and preemption handling for a connected processor in the system."

	This class describes the CPU interface
*/
#ifndef INTERRUPT_ARM_GIC_CPU_H_
#define INTERRUPT_ARM_GIC_CPU_H_

#include <stdint.h>

/*
	class ATOSE_interrupt_arm_gic_cpu
	---------------------------------
	See page 4-4 to 4-5 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
	The long names are use for clarity,  the offsets are provided for verification purposes and the short names (in
	brackets) are given for look-up purposes.
*/
class ATOSE_interrupt_arm_gic_cpu
{
uint32_t cpu_interface_control_register;							// 0x00 (ICCICR)
uint32_t interrupt_priority_mask_register;						// 0x04 (ICCPMR)
uint32_t binary_point_register;										// 0x08 (ICCBPR)
uint32_t interrupt_acknowledge_register;							// 0x0C (ICCIAR)
uint32_t end_of_interrupt_register;									// 0x10 (ICCEOIR)
uint32_t running_priority_register;									// 0x14 (ICCRPR)
uint32_t highest_pending_interrupt_register;						// 0x18 (ICCHPIR)
uint32_t aliased_binary_point_register;							// 0x1C (ICCABPR)
uint32_t reserved1[8];
uint32_t implementation_defined_registers[36];
uint32_t reserved2[11];
uint32_t cpu_interface_dentification_register;					// 0xFC (ICCIIDR)
} ;

#endif
