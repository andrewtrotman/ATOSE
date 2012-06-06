/*
	INTERRUPTS.C
	------------
*/
#include "interrupts.h"
#include "registers.h"
#include "pic.h"

/*
	__CS3_ISR_IRQ()
	---------------
*/
void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq()
{
/*
	FIX:MAKE SURE THE REGISTERS ARE ALL SAVED (WHICH AT THE MOMENT THEY ARE NOT)
*/
ATOSE_device_driver *device_driver;

device_driver = (ATOSE_device_driver *)*ATOSE_pic::PIC_vector_address_register;
if (device_driver != 0)
	device_driver->acknowledge();

*ATOSE_pic::PIC_vector_address_register = 0;
}

/*
	__CS3_ISR_SWI()
	---------------
*/
void __attribute__ ((interrupt("SWI"))) __cs3_isr_swi(void)
{
/*
	FIX:MAKE SURE THE REGISTERS ARE ALL SAVED (WHICH AT THE MOMENT THEY ARE NOT)
*/
static uint32_t val = 0;
uint32_t swi_id;
static const uint32_t ATOSE_SWI = 0x6174;

/*
	First we need to determine whether or no the swi is for us.  We do this by getting the SWI number,
	that number is stored in the instruction just executed, which is stored at R14.  So we subtract 4 from
	R14 to get the instruction then turn off the top bits to get the number
*/
asm volatile ("ldr %0, [r14,#-4]" : "=r"(swi_id));

swi_id &= 0x00FFFFFF;
if (swi_id == ATOSE_SWI)
	{
	val++;
	asm volatile ("mov r0, %[val]" : : [val]"r"(val));
	}
else
	asm volatile ("mov r0, #0");
}

/*
	__CS3_ISR_PABORT()
	------------------
*/
void __attribute__ ((interrupt("ABORT"))) __cs3_isr_pabort(void)
{
ATOSE_registers regs;

ATOSE_registers_get(&regs);
}

/*
	__CS3_ISR_DABORT()
	------------------
*/
void __attribute__ ((interrupt("ABORT"))) __cs3_isr_dabort(void)
{
}

/*
	__CS3_ISR_UNDEF()
	-----------------
*/
void __attribute__ ((interrupt("UNDEF"))) __cs3_isr_undef(void)
{
}

/*
	__CS3_ISR_RESERVED()
	--------------------
	This can't happen as this interrupt is reserved
*/
void __attribute__ ((interrupt)) __cs3_isr_reserved(void)
{
}

/*
	__CS3_ISR_FIQ()
	---------------
*/
void __attribute__ ((interrupt("FIQ"))) __cs3_isr_fiq(void)
{
}

