/*
	KEYBOARD_MOUSE_INTERFACE.C
	--------------------------
	ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050)
*/
#include "keyboard_mouse_interface.h"

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::INIT()
	--------------------------------------
*/
void ATOSE_keyboard_mouse_interface::init(unsigned char *KMI_base_address)
{
/*
	Construct my parent object then constuct me
*/
ATOSE_IO_serial::init();

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
uint8_t got;

got = *KMI_data_register;
push(&got);
}

/*
	ATOSE_KEYBOARD_MOUSE_INTERFACE::WRITE()
	---------------------------------------
*/
uint32_t ATOSE_keyboard_mouse_interface::write(const uint8_t *buffer, uint32_t  bytes)
{
uint32_t  current;

for (current = 0; current < bytes; current++)
	*KMI_data_register = buffer[current];

return bytes;
}
