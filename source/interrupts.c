/*
	INTERRUPTS.C
	------------
*/
#include "interrupts.h"
#include "atose.h"
#include "api_atose.h"

#ifdef IMX233
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsicoll.h"
#endif

/*
	__CXA_PURE_VIRTUAL()
	--------------------
*/
int __cxa_pure_virtual(void)
{
return 0;
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
uint32_t ATOSE_isr_irq(ATOSE_registers *registers)
{
#ifdef IMX233

	ATOSE_device_driver *device_driver;

	uint32_t got = HW_ICOLL_STAT_RD();										// get the interrupt number (in case we need it)

	device_driver = *((ATOSE_device_driver **)HW_ICOLL_VECTOR_RD());		// tell the CPU that we've entered the interrupt service routine and get the ISR address
	
	if (device_driver != 0)
		device_driver->acknowledge();

	ATOSE *os = ATOSE::get_global_entry_point();
	os->io << "[" << got << "]";

	HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);			// finished processing the interrupt

#else

	ATOSE_device_driver *device_driver;

	device_driver = (ATOSE_device_driver *)*ATOSE_pic_pl190::PIC_vector_address_register;
	if (device_driver != 0)
		device_driver->acknowledge();

	ATOSE *os = ATOSE::get_global_entry_point();
	os->io << ".";

	*ATOSE_pic_pl190::PIC_vector_address_register = 0;

#endif
}

/*
	__CS3_ISR_PABORT()
	------------------
*/
void __attribute__ ((interrupt("ABORT"))) __cs3_isr_pabort(void)
{
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

/*
	ATOSE_ISR_SWI()
	---------------
*/
uint32_t ATOSE_isr_swi(ATOSE_registers *registers)
{
static uint32_t val = 0;
ATOSE_IO *object;
ATOSE *os = ATOSE::get_global_entry_point();

/*
	First we need to determine whether or no the swi is for us.  We do this by getting the SWI number,
	that number is stored in the instruction just executed, which is stored at R14.  So we subtract 4 from
	R14 to get the instruction then turn off the top bits to get the number
*/
if (( (*(uint32_t *)(registers->r14_current - 4)) & 0x00FFFFFF) != ATOSE_SWI)
	return 0;

/*
	The SWI is for us.
	First get the object
*/
switch (registers->r0)
	{
#ifndef IMX233

	case ATOSE_API::id_object_keyboard:
//		os->io << "[Keyboard]\n";
		object = &os->keyboard;
		break;
	case ATOSE_API::id_object_mouse:
//		os->io << "[Mouse]\n";
		object = &os->mouse;
		break;
#endif
	case ATOSE_API::id_object_serial:
//		os->io << "[Serial]\n";
		object = &os->io;
		break;
	default:
//		os->io << "["<< registers->r0 << "]\n";
		return 0;
	}

/*
	Now get the method
*/
switch (registers->r1)
	{
	case ATOSE_API::id_function_read_byte:
		{
		uint8_t byte;

		registers->r0 = object->read_byte(&byte);
		registers->r1 = byte;
		break;
		}
	case ATOSE_API::id_function_write_byte:
		{
		registers->r0 = object->write_byte(registers->r2);
		break;
		}
	default:
		return 0;
	}

return 1;
}
