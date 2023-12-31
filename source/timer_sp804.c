/*
	TIMER_SP804.C
	-------------
	ARM Dual-Timer Module (SP804)
*/
#include <stdint.h>
#include "timer_sp804.h"


/*
	base address of the timer interrupt controller
*/
unsigned char *ATOSE_timer_sp804::timer_base_address = (unsigned char *)0x101E2000;

/*
	programmable interrupt controller registers
*/
volatile uint32_t *ATOSE_timer_sp804::timer_0_load = (uint32_t *)(timer_base_address + 0x00);
volatile uint32_t *ATOSE_timer_sp804::timer_0_value = (uint32_t *)(timer_base_address + 0x04);
volatile uint32_t *ATOSE_timer_sp804::timer_0_control = (uint32_t *)(timer_base_address + 0x08);
volatile uint32_t *ATOSE_timer_sp804::timer_0_intclr = (uint32_t *)(timer_base_address + 0x0c);
volatile uint32_t *ATOSE_timer_sp804::timer_0_ris = (uint32_t *)(timer_base_address + 0x10);
volatile uint32_t *ATOSE_timer_sp804::timer_0_mis = (uint32_t *)(timer_base_address + 0x14);
volatile uint32_t *ATOSE_timer_sp804::timer_0_bgload = (uint32_t *)(timer_base_address + 0x18);

/*
	ATOSE_TIMER_SP804::ENABLE()
	---------------------------
*/
void ATOSE_timer_sp804::enable(void)
{
*timer_0_load = (unsigned long)0x2000;
*timer_0_control = (*timer_0_control & 0xFFFFFF10) | 0xE0;
}

/*
	ATOSE_TIMER_SP804::DISABLE()
	----------------------------
*/
void ATOSE_timer_sp804::disable(void)
{
*timer_0_control &= 0xFFFFFF10;
}


/*
	ATOSE_TIMER_SP804::ACKNOWLEDGE()
	--------------------------------
*/
void ATOSE_timer_sp804::acknowledge(void)
{
*timer_0_intclr = 0;
}
