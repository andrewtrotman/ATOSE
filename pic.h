/*
	PIC.H
	-----
	Programmable Interrupt controller
	See the ARMPL190 VIC Technical Reference Manual for more information
*/
#ifndef PIC_H_
#define PIC_H_

#include <stdint.h>

extern "C"
{
void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq();
}	

class ATOSE_device_driver;
/*
	class ATOSE_PIC
	---------------
*/
class ATOSE_pic
{
friend
	void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq();

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

	static const uint32_t NO_SECONDARY = 0xFFFFFFFF;

private:
	uint32_t next_interrupt_id;

public:
	ATOSE_pic();
	virtual ~ATOSE_pic();

	void enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary = NO_SECONDARY);

#ifdef NEVER
	void timer_enable(void);
	void timer_enter(void);
	void timer_exit(void);

	void keyboard_enable(void);
#endif
} ;

#endif /* PIC_H_ */
