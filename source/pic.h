/*
	PIC.H
	-----
	Programmable Interrupt controller
	See the ARMPL190 VIC Technical Reference Manual for more information
*/
#ifndef PIC_H_
#define PIC_H_

#include <stdint.h>
#include "interrupts.h"
#include "device_driver.h"

class ATOSE_device_driver;

/*
	class ATOSE_PIC
	---------------
*/
class ATOSE_pic : public ATOSE_device_driver
{
friend
	void __cs3_isr_irq(void);

private:
	/*
		Base address of the primary interrupt controller
	*/
	static unsigned char *PIC_base_address;

	/*
		Each of the seperate registers on the primary interrupt controller
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

	/*
		Base address of the primary interrupt controller
	*/
	static unsigned char *SEC_base_address;

	/*
		Each of the seperate registers on the secondary interrupt controller
	*/
	static volatile uint32_t *SEC_status;
	static volatile uint32_t *SEC_RAW_status;
	static volatile uint32_t *SEC_enable;
	static volatile uint32_t *SEC_enclr;
	static volatile uint32_t *SEC_softintset;
	static volatile uint32_t *SEC_softintclr;
	static volatile uint32_t *SEC_picenable;
	static volatile uint32_t *SEC_picenclr;

	/*
		Sentinal constants
	*/
	static const uint32_t NO_SECONDARY = 0xFFFFFFFF;

private:
	uint32_t next_interrupt_id;
	ATOSE_device_driver *secondary_table[32];

public:
	ATOSE_pic() {}
	virtual void init(void);

	virtual void enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary = NO_SECONDARY);
	virtual void acknowledge(void);
} ;

#endif /* PIC_H_ */
