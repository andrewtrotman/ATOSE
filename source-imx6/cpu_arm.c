/*
	CPU_ARM.C
	---------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>
#include "cpu_arm.h"

/*
	ATOSE_CPU_ARM::GET_CPSR()
	-------------------------
*/
uint32_t ATOSE_cpu_arm::get_cpsr(void)
{
uint32_t answer;

asm volatile
	(
	"mrs %0,CPSR"
	:"=r" (answer)
	);

return answer;
}

/*
	ATOSE_CPU_ARM::SET_CPSR()
	-------------------------
*/
void ATOSE_cpu_arm::set_cpsr(uint32_t save_cpsr)
{
asm volatile
	(
	"msr CPSR_cxsf, %0"
	:
	:"r"(save_cpsr)
	);
}
