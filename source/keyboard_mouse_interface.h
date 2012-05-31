/*
	KEYBOARD_MOUSE_INTERFACE.H
	--------------------------
	ARM PrimeCell PS2 Keyboard/Mouse Interface (PL050)
*/
#ifndef KEYBOARD_MOUSE_INTERFACE_H_
#define KEYBOARD_MOUSE_INTERFACE_H_

#include <stdint.h>
#include "io_serial.h"

/*
	class ATOSE_KEYBOARD_MOUSE_INTERFACE
	------------------------------------
*/
class ATOSE_keyboard_mouse_interface : public ATOSE_IO_serial
{
private:
	volatile uint32_t *KMI_control_register;
	volatile uint32_t *KMI_status_register;
	volatile uint32_t *KMI_data_register;
	volatile uint32_t *KMI_clock_divisor_register;
	volatile uint32_t *KMI_interrupt_identification_register;

public:
	ATOSE_keyboard_mouse_interface() : ATOSE_IO_serial() {}
	virtual void init(unsigned char *KMI_base_address);

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);

	virtual int write(const char *buffer, int bytes);
} ;

#endif /* KEYBOARD_MOUSE_INTERFACE_H_ */
