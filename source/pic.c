/*
	PIC.C
	-----
	PrimeCell Vectored Interrupt Controller (PL190)
*/
#include <stdio.h>
#include "pic.h"
#include "device_driver.h"

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
volatile uint32_t *ATOSE_pic::PIC_vector_address_registers = (uint32_t *)(PIC_base_address + 0x100) + 1;			// FIX: Why + 1?  QEMU bug?
volatile uint32_t *ATOSE_pic::PIC_vector_control_registers = (uint32_t *)(PIC_base_address + 0x200);

unsigned char *ATOSE_pic::SEC_base_address = (unsigned char *)0x10003000;

volatile uint32_t *ATOSE_pic::SEC_status = (uint32_t *)(SEC_base_address + 0x00);
volatile uint32_t *ATOSE_pic::SEC_RAW_status = (uint32_t *)(SEC_base_address + 0x04);
volatile uint32_t *ATOSE_pic::SEC_enable = (uint32_t *)(SEC_base_address + 0x08);
volatile uint32_t *ATOSE_pic::SEC_enclr = (uint32_t *)(SEC_base_address + 0x0C);
volatile uint32_t *ATOSE_pic::SEC_softintset = (uint32_t *)(SEC_base_address + 0x10);
volatile uint32_t *ATOSE_pic::SEC_softintclr = (uint32_t *)(SEC_base_address + 0x14);
volatile uint32_t *ATOSE_pic::SEC_picenable = (uint32_t *)(SEC_base_address + 0x20);
volatile uint32_t *ATOSE_pic::SEC_picenclr = (uint32_t *)(SEC_base_address + 0x24);

/*
	ATOSE_PIC::INIT()
	-----------------
*/
void ATOSE_pic::init(void)
{
uint32_t current;

/*
	If we get an interrupt from an unknown source we'll get a passed to the
	interrupt service routine
*/
*PIC_default_vector_address_register = 0;

/*
	Send everyhting to IRQ
*/
*PIC_interrupt_select_register = 0;								// everything to IRQ

/*
	The next interrupt we see will be ID = 0
*/
next_interrupt_id = 0;

/*
	Initiialsie the secondary pointer table
*/
for (current = 0; current < sizeof(secondary_table) / sizeof(*secondary_table); current++)
	secondary_table[current] = 0;
}

/*
	ATOSE_PIC::ENABLE()
	-------------------
*/
void ATOSE_pic::enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary)
{	
/*
	Vectors are ordered in creation order
*/
PIC_vector_address_registers[next_interrupt_id] = (uint32_t)driver;			// pointer to the driver object
PIC_vector_control_registers[next_interrupt_id] = (uint32_t)0x20 | primary; 	// enable vectored interrupts for the given interrupt line

/*
	Now enable the primary interrupt controller
*/
*PIC_interrupt_enable_register |= 1 << primary;					// enable the interrupt

/*
	If the secondary is to be used then enable that too
*/
if (secondary != NO_SECONDARY)
	{
	*SEC_enable |= 1 << secondary;									// enable the interrupt
	secondary_table[secondary] = driver;
	PIC_vector_address_registers[next_interrupt_id] = (uint32_t)this;
	}

next_interrupt_id++;
}

/*
	ATOSE_PIC::ACKNOWLEDGE()
	------------------------
	This gets called for the secondary interrupt controller
*/
void ATOSE_pic::acknowledge(void)
{
uint32_t source, current;

/*
	Get a list of the interrupt sources
*/
source = *SEC_status;

/*
	Now call the handlers	(several bits might be set)
*/
for (current = 0; current < 32; current++)
	{
	if ((source & (1 << current)) != 0)
		secondary_table[current]->acknowledge();
	}
}