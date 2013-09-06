/*
	INTERRUPT_ARM_GIC_DISTRIBUTOR.H
	-------------------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	According to page 2-2 of  "ARM Generic Interrupt Controller Architecture Specification Version 1"

	"This GIC architecture splits logically into a Distributor block and one or more CPU interface blocks, as
	Figure 2-1 on page 2-3 shows:
	Distributor 		This performs interrupt prioritization and distribution to the CPU interfaces that connect to the processors in the system.
	CPU interfaces 	Each CPU interface performs priority masking and preemption handling for a connected processor in the system."

	This class describes the distributor interface
*/
#ifndef INTERRUPT_ARM_GIC_DISTRIBUTOR_H_
#define INTERRUPT_ARM_GIC_DISTRIBUTOR_H_

#include <stdint.h>

/*
	class ATOSE_INTERRUPT_ARM_GIC_DISTRIBUTOR
	-----------------------------------------
	See page 4-2 to 4-4 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
	There should be one of these per CPU. The long names are preferred for clarity, the offset address is given
	for verification purposes and the short register name is given for look-up purposes.
*/
class ATOSE_interrupt_arm_gic_distributor
{
public:
	uint32_t distributor_control_register;							      			// 0x0000 (ICDDCR)
	uint32_t interrupt_controller_type_register;					      			// 0x0004 (ICDICTR)
	uint32_t distributor_implementer_identification_register;					// 0x0008 (ICDIIDR)
	uint32_t reserved1[29];
	uint32_t interrupt_security_registers[8];						   				// 0x0080 (ICDISR)
	uint32_t reserved2[24];
	uint32_t interrupt_set_enable_registers[32];					      			// 0x0100 (ICDISER)
	uint32_t interrupt_clear_enable_registers[32];					   			// 0x0180 (ICDICER)
	uint32_t interrupt_set_pending_registers[32];					   			// 0x0200 (ICDISPR)
	uint32_t interrupt_clear_pending_registers[32];				      			// 0x0280 (ICDICPR)
	uint32_t active_bit_registers[32];								      			// 0x0300 (ICDABR)
	uint32_t reserved3[32];
	uint8_t interrupt_priority_registers[255 * sizeof(uint32_t)];				// 0x0400 (ICDIPR)
	uint32_t reserved4;
	uint8_t interrupt_processor_targets_registers[255 * sizeof(uint32_t)];	// 0x0800 (ICDIPTR)
	uint32_t reserved5;
	uint32_t interrupt_configuration_registers[64];									// 0x0C00 (ICDICFR)
	uint32_t implementation_defined_registers[64];
	uint32_t reserved6[64];
	uint32_t software_generated_interrupt_register;									// 0x0F00 (ICDSGIR)
	uint32_t reserved7[51];
	/*
		The next 12 32-bit words are implementation defined identification_registers
	*/
	uint32_t peripheral_id4;																// 0x0FD0 (ICPIDR4)
	uint32_t peripheral_id5;																// 0x0FD4 (ICPIDR5)
	uint32_t peripheral_id6;																// 0x0FD8 (ICPIDR6)
	uint32_t peripheral_id7;																// 0x0FDC (ICPIDR7)
	uint32_t peripheral_id0;																// 0x0FE0 (ICPIDR0)
	uint32_t peripheral_id1;																// 0x0FE4 (ICPIDR1)
	uint32_t peripheral_id2;																// 0x0FE8 (ICPIDR2)
	uint32_t peripheral_id3;																// 0x0FEC (ICPIDR3)
	uint32_t component_id0;																	// 0x0FF0 (ICCIDR0)
	uint32_t component_id1;																	// 0x0FF4 (ICCIDR1)
	uint32_t component_id2;																	// 0x0FF8 (ICCIDR2)
	uint32_t component_id3;																	// 0x0FFC (ICCIDR3)
} __attribute__ ((packed));

#endif
