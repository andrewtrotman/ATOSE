/*
	KEYBOARD_MOUSE_INTERFACE.C
	--------------------------
	ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050)
*/
#include "keyboard_mouse_interface.h"

/*
	KMI base address
*/
volatile unsigned char *ATOSE_keyboard_mouse_interface::KMI_base_address = (unsigned char *)0x10006000;		// keyboard

/*
	KMI controller registers
*/
volatile uint32_t *ATOSE_keyboard_mouse_interface::KMI_control_register = (uint32_t *)(KMI_base_address + 0x00);
volatile uint32_t *ATOSE_keyboard_mouse_interface::KMI_status_register = (uint32_t *)(KMI_base_address + 0x04);
volatile uint32_t *ATOSE_keyboard_mouse_interface::KMI_data_register = (uint32_t *)(KMI_base_address + 0x08);
volatile uint32_t *ATOSE_keyboard_mouse_interface::KMI_clock_divisor_register = (uint32_t *)(KMI_base_address + 0x0C);
volatile uint32_t *ATOSE_keyboard_mouse_interface::KMI_interrupt_identification_register = (uint32_t *)(KMI_base_address + 0x10);


/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::ATOSE_KEYBOARD_MOUSE_INTERFACE()
	----------------------------------------------------------------
*/
ATOSE_keyboard_mouse_interface::ATOSE_keyboard_mouse_interface()
{
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::ENABLE()
	----------------------------------------
*/
void ATOSE_keyboard_mouse_interface::enable(void)
{
*KMI_control_register = *KMI_control_register | 0x14;			// turn on and enable recieve interrupts
}

