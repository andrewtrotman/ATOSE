/*
	CPU.C
	-----
*/
#include "cpu.h"

/*
	ATOSE_CPU::GET_CPSR()
	---------------------
*/
int ATOSE_cpu::get_cpsr(void)
{ 
int answer;

asm volatile
	(
	"mrs %0,CPSR"
	:"=r" (answer)
	:
	:
	);

return answer;
}

/*
	ATOSE_CPU::SET_CPSR()
	---------------------
*/
void ATOSE_cpu::set_cpsr(uint32_t save_cpsr)
{
asm volatile
	(
	"msr CPSR_cxsf, %0"
	:
	:"r"(save_cpsr)
	:
	);
}

/*
	ATOSE_CPU::ENABLE_IRQ()
	-----------------------
*/
void ATOSE_cpu::enable_IRQ(void)
{
set_cpsr(get_cpsr() & ~0x80);
}

/*
	ATOSE_CPU::DISABLE_IRQ()
	------------------------
*/
void ATOSE_cpu::disable_IRQ(void)
{
set_cpsr(get_cpsr() | 0x80);
}

/*
	ATOSE_CPU::ENTER_USER_MODE()
	----------------------------
*/
void ATOSE_cpu::enter_user_mode(void)
{
set_cpsr((get_cpsr() & ~0x01F) | 0x10);
}