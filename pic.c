/*
	PIC.C
	-----
	PrimeCell Vectored Interrupt Controller (PL190)
*/
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

/*
	Globabl (to be removed)
*/
volatile uint32_t number_of_ticks = 0;

/*
	__CS3_ISR_IRQ()
	---------------
*/
extern "C"
{
	void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq()
	{
	ATOSE_device_driver *device_driver;

	device_driver = (ATOSE_device_driver *)*ATOSE_pic::PIC_vector_address_register;
	if (device_driver != 0)
		device_driver->acknowledge();

	number_of_ticks++;

	*ATOSE_pic::PIC_vector_address_register = 0;
	}
}

/*
	ATOSE_PIC::ATOSE_PIC()
	----------------------
*/
ATOSE_pic::ATOSE_pic()
{
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
}

/*
	ATOSE_PIC::~ATOSE_PIC()
	-----------------------
*/
ATOSE_pic::~ATOSE_pic()
{
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
next_interrupt_id++;
/*
	Now enable the interrupt
*/
*PIC_interrupt_enable_register |= (1 << primary);					// enable the interrupt
}

#ifdef NEVER
/*
	ATOSE_PIC::TIMER_ENABLE()
	-------------------------
*/
void ATOSE_pic::timer_enable(void)
{
/*
	Timer interrupt	(0x04)
*/
PIC_vector_address_registers[0] = 1;								// the interrupt service rountine id
PIC_vector_control_registers[0] = (uint32_t)0x24;					// enable vectored interrupts for source 0x04 (the timer)

/*
	UART 0 (Serial Port) interrupt (0x0C)
*/
PIC_vector_address_registers[1] = 2;								// the interrupt service rountine id
PIC_vector_control_registers[1] = (uint32_t)0x2C;					// enable vectored interrupts for source 0x0C (UART 0)

/*
	Now enable the interrupts
*/
*PIC_interrupt_enable_register = (1 << 4) | (1 << 0x0C);			// enable clock and UART 0
}

/*
	ATOSE_PIC::TIMER_ENTER()
	------------------------
*/
void ATOSE_pic::timer_enter(void)
{
uint32_t isr;

isr = *PIC_vector_address_register;					// enter processing mode
}


/*
	ATOSE_PIC::TIMER_EXIT()
	-----------------------
*/
void ATOSE_pic::timer_exit(void)
{
*PIC_vector_address_register = 0;						// clear the interrupt (exit processing mode)
}


/*
	ATOSE_PIC::KEYBOARD_ENABLE()
	----------------------------
*/
void ATOSE_pic::keyboard_enable(void)
{
volatile uint32_t *SIC_interrupt_enable_register = (uint32_t *)0x10003008;

*PIC_interrupt_enable_register |= 0x80000000;			// secondary interrupt controller (0x80000000)
*SIC_interrupt_enable_register = 0x08;					// enable PS2 keyboard
}

#endif