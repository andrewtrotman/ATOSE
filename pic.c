/*
	PIC.C
	-----
	PrimeCell Vectored Interrupt Controller (PL190)
*/
#include "pic.h"

unsigned char *ATOSE_pic::PIC_base_address = (unsigned char *)0x10140000;

volatile uint32_t *ATOSE_pic::PIC_IRQ_status_register = (uint32_t *)(PIC_base_address + 0x00);
volatile uint32_t *ATOSE_pic::PIC_FIRQ_status_register = (uint32_t *)(PIC_base_address + 0x04);
volatile uint32_t *ATOSE_pic::PIC_RAW_status_register = (uint32_t *)(PIC_base_address + 0x08);
volatile uint32_t *ATOSE_pic::PIC_interrupt_select_register = (uint32_t *)(PIC_base_address + 0x0C);
volatile uint32_t *ATOSE_pic::PIC_interrupt_enable_register = (uint32_t *)(PIC_base_address + 0x10);
volatile uint32_t *ATOSE_pic::PIC_interrupt_enable_clear_register = (uint32_t *)(PIC_base_address + 0x14);
volatile uint32_t *ATOSE_pic::PIC_software_interrupt_register = (uint32_t *)(PIC_base_address + 0x18);
volatile uint32_t *ATOSE_pic::PIC_software_interrupt_clear_register = (uint32_t *)(PIC_base_address + 0x1C);
volatile uint32_t *ATOSE_pic::PIC_protection_enable_register = (uint32_t *)(PIC_base_address + 0x20);
volatile uint32_t *ATOSE_pic::PIC_vector_address_register = (uint32_t *)(PIC_base_address + 0x30);
volatile uint32_t *ATOSE_pic::PIC_default_vector_address_register = (uint32_t *)(PIC_base_address + 0x34);
volatile uint32_t *ATOSE_pic::PIC_vector_address_registers = (uint32_t *)(PIC_base_address + 0x100);
volatile uint32_t *ATOSE_pic::PIC_vector_control_registers = (uint32_t *)(PIC_base_address + 0x200);
volatile uint32_t *ATOSE_pic::PIC_peripheral_id_register = (uint32_t *)(PIC_base_address + 0xFE0);



/*
	ATOSE_PIC::ATOSE_PIC()
	----------------------
*/
ATOSE_pic::ATOSE_pic()
{
}

/*
	ATOSE_PIC::TIMER_ENABLE()
	-------------------------
*/
void ATOSE_pic::timer_enable(void)
{
*PIC_interrupt_enable_register = 0x10;
}

/*
	ATOSE_PIC::TIMER_ENTER()
	------------------------
*/
void ATOSE_pic::timer_enter(void)
{
isr = *PIC_vector_address_register;
}


/*
	ATOSE_PIC::TIMER_EXIT()
	-----------------------
*/
void ATOSE_pic::timer_exit(void)
{
*PIC_vector_address_register = isr;
}