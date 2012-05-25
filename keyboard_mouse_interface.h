/*
	KEYBOARD_MOUSE_INTERFACE.H
	--------------------------
	ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050)
*/
#ifndef KEYBOARD_MOUSE_INTERFACE_H_
#define KEYBOARD_MOUSE_INTERFACE_H_

#include <stdint.h>

/*
	class ATOSE_KEYBOARD_MOUSE_INTERFACE
	------------------------------------
*/
class ATOSE_keyboard_mouse_interface
{
private:
	static volatile unsigned char *KMI_base_address;

	static volatile uint32_t *KMI_control_register;
	static volatile uint32_t *KMI_status_register;
	static volatile uint32_t *KMI_data_register;
	static volatile uint32_t *KMI_clock_divisor_register;
	static volatile uint32_t *KMI_interrupt_identification_register;

public:
	ATOSE_keyboard_mouse_interface();

	void enable(void);
} ;


#endif /* KEYBOARD_MOUSE_INTERFACE_H_ */
