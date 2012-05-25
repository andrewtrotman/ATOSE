/*
	PIC.H
	-----
	Programmable Interrupt controller
	See the ARMPL190 VIC Technical Reference Manual for more information
*/
#ifndef PIC_H_
#define PIC_H_

#include <stdint.h>

/*
	class ATOSE_PIC
	---------------
*/
class ATOSE_pic
{
private:
	/*
		Base address of the Programmable Interrupt Controller
	*/
	static unsigned char *PIC_base_address;

	/*
		Each of the seperate registers
	*/
	static volatile uint32_t *PIC_IRQ_status_register;
	static volatile uint32_t *PIC_FIRQ_status_register;
	static volatile uint32_t *PIC_RAW_status_register;
	static volatile uint32_t *PIC_interrupt_select_register;
	static volatile uint32_t *PIC_interrupt_enable_register;
	static volatile uint32_t *PIC_interrupt_enable_clear_register;
	static volatile uint32_t *PIC_software_interrupt_register;
	static volatile uint32_t *PIC_software_interrupt_clear_register;
	static volatile uint32_t *PIC_protection_enable_register;
	static volatile uint32_t *PIC_vector_address_register;
	static volatile uint32_t *PIC_default_vector_address_register;
	static volatile uint32_t *PIC_vector_address_registers;
	static volatile uint32_t *PIC_vector_control_registers;
	static volatile uint32_t *PIC_peripheral_id_register;

public:
	ATOSE_pic();

	void timer_enable(void);
	void timer_enter(void);
	void timer_exit(void);

	void keyboard_enable(void);
} ;

#endif /* PIC_H_ */
