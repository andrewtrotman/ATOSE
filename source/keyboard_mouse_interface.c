/*
	KEYBOARD_MOUSE_INTERFACE.C
	--------------------------
	ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050)
*/
#include <stdio.h>
#include "keyboard_mouse_interface.h"

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::ATOSE_KEYBOARD_MOUSE_INTERFACE()
	----------------------------------------------------------------
*/
ATOSE_keyboard_mouse_interface::ATOSE_keyboard_mouse_interface(unsigned char *KMI_base_address) : ATOSE_IO_serial()
{
/*
	KMI controller registers
*/
KMI_control_register = (uint32_t *)(KMI_base_address + 0x00);
KMI_status_register = (uint32_t *)(KMI_base_address + 0x04);
KMI_data_register = (uint32_t *)(KMI_base_address + 0x08);
KMI_clock_divisor_register = (uint32_t *)(KMI_base_address + 0x0C);
KMI_interrupt_identification_register = (uint32_t *)(KMI_base_address + 0x10);
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::~ATOSE_KEYBOARD_MOUSE_INTERFACE()
	-----------------------------------------------------------------
*/
ATOSE_keyboard_mouse_interface::~ATOSE_keyboard_mouse_interface()
{
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::ENABLE()
	----------------------------------------
*/
void ATOSE_keyboard_mouse_interface::enable(void)
{
/*
	Enable the KMI hardware and the keyboard interrupt on recieve
*/
*KMI_control_register = *KMI_control_register | 0x14;
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::DISABLE()
	-----------------------------------------
*/
void ATOSE_keyboard_mouse_interface::disable(void)
{
/*
	Disable interrupts
*/
*KMI_control_register = 0;
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::ACKNOWLEDGE()
	---------------------------------------------
*/
void ATOSE_keyboard_mouse_interface::acknowledge(void)
{
unsigned char got;

got = *KMI_data_register;
push(&got);
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::WRITE()
	---------------------------------------
*/
int ATOSE_keyboard_mouse_interface::write(const char *buffer, int bytes)
{
int current;

for (current = 0; current < bytes; current++)
	*KMI_data_register = buffer[current];

return bytes;
}
