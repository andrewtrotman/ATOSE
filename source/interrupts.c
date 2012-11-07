/*
	INTERRUPTS.C
	------------
*/
#include "interrupts.h"
#include "process.h"
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

void __cs3_isr_undef(void)
{ 
ATOSE *os = ATOSE::get_global_entry_point();
os->heap.assume_identity();
os->io << "\r\nUNDEFINED INTERRUPT\r\n";
for (;;);
}

void __cs3_isr_pabort(void)
{ 
ATOSE *os = ATOSE::get_global_entry_point();
os->heap.assume_identity();
os->io << "\r\nP-ABORT INTERRUPT\r\n";
for (;;);
}

void __cs3_isr_dabort(void)
{ 
ATOSE *os = ATOSE::get_global_entry_point();
os->heap.assume_identity();
os->io << "\r\nD-ABORT INTERRUPT\r\n";
for (;;);
}

void __cs3_isr_reserved(void)
{ 
ATOSE *os = ATOSE::get_global_entry_point();
os->heap.assume_identity();
os->io << "\r\nRESERVED INTERRUPT\r\n";
for (;;);
}

void __cs3_isr_fiq(void)
{ 
ATOSE *os = ATOSE::get_global_entry_point();
os->heap.assume_identity();
os->io << "\r\nFIQ INTERRUPT\r\n";
for (;;);
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
uint32_t ATOSE_isr_irq(ATOSE_registers *registers)
{
/*
	Get a handle to the ATOSE object
*/
ATOSE *os = ATOSE::get_global_entry_point();

/*
	If we're running a process then copy the registers into its register space
	this way if we cause a context switch then we've not lost anything
*/
if (os->scheduler.get_current_process() != NULL)
	memcpy(&os->scheduler.get_current_process()->execution_path.registers, registers, sizeof(*registers));

/*
	get into the ATOSE address space
*/
os->heap.assume_identity();

/*
	Handle the interrupt by calling the device driver's ack method
*/
#ifdef IMX233

	ATOSE_device_driver *device_driver;

//	uint32_t got = HW_ICOLL_STAT_RD();										// get the interrupt number (in case we need it)

	device_driver = *((ATOSE_device_driver **)HW_ICOLL_VECTOR_RD());		// tell the CPU that we've entered the interrupt service routine and get the ISR address
	
	if (device_driver != 0)
		device_driver->acknowledge();

//	os->io << "[" << got << "]";

	HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);			// finished processing the interrupt

#else

	ATOSE_device_driver *device_driver;

	device_driver = (ATOSE_device_driver *)*ATOSE_pic_pl190::PIC_vector_address_register;
	if (device_driver != 0)
		device_driver->acknowledge();

	*ATOSE_pic_pl190::PIC_vector_address_register = 0;

#endif

/*
	Context switch
*/
if (os->scheduler.get_next_process() != NULL)
	{
//	os->io.hex();
//	os->io << "->" << os->scheduler.get_current_process()->execution_path.registers.r14_current << " SPACE:" << (uint32_t)&os->scheduler.get_current_process()->address_space;

	/*
		Set the registers so that we fall back to the next context
	*/
	memcpy(registers, &os->scheduler.get_current_process()->execution_path.registers, sizeof(*registers));

	/*
		Set the address space to fall back to the next context
	*/
	os->heap.assume(&os->scheduler.get_current_process()->address_space);
	}

return 0;
}

/*
	ATOSE_ISR_SWI()
	---------------
*/
uint32_t ATOSE_isr_swi(ATOSE_registers *registers)
{
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
	switch to identity page table
*/
os->heap.assume_identity();

//	os->io << "[" << (char)registers->r0 << "->" << (char)registers->r1 << (const uint8_t)registers->r2 << "]";

/*
	The SWI is for us.
	First get the object
*/
switch (registers->r0)
	{
#ifdef QEMU
	case ATOSE_API::id_object_keyboard:
		object = &os->keyboard;
		break;
	case ATOSE_API::id_object_mouse:
		object = &os->mouse;
		break;
#endif
	case ATOSE_API::id_object_serial:
		object = &os->io;
		break;
	case ATOSE_API::id_object_process_manager:
		object = &os->scheduler;
		break;
	default:
		os->io << "Fatal (1)";
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
	case ATOSE_API::id_function_write_block:
		{
		/*
			At present this only works on the process object - to be fixed later
			When writing to the procss_manager object this loads an ELF file
			sets up the address space and executes it.
		*/
		object->write((uint8_t *)registers->r2, registers->r3);
		break;
		}
	default:
		os->io << "Fatal (2)";
		return 0;
	}

/*
	Go back into the caller's address space
*/
os->heap.assume(&os->scheduler.get_current_process()->address_space);

return 1;
}
