/*
	ATOSE.C
	-------
*/
#include "atose.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"

extern ATOSE *ATOSE_addr;

/*
	ATOSE::ATOSE()
	--------------
*/
ATOSE::ATOSE()
{
/*
	First things first... put a pointer to me at the end of the interrupt space so that
	we can at ant time get a pointer to this object (by calling ATOSE::get_global_entry_point()
*/
ATOSE_addr = this;

/*
	Initialise each of the essential core objects
*/
#ifdef IMX233
	stack.init();
	cpu.init();
	pic.init();
	io.init();
	timer.init();
	disk.init();
#else
	stack.init();
	cpu.init();
	pic.init();
	io.init();
	timer.init();
	keyboard.init((unsigned char *)0x10006000);
	mouse.init((unsigned char *)0x10007000);
#endif

/*
	Now join them together to form a working system
*/
#ifdef IMX233
	cpu.enable_IRQ();

//	pic.enable(&timer, VECTOR_IRQ_RTC_1MSEC);
//	timer.enable();

	pic.enable(&io, VECTOR_IRQ_DEBUG_UART);
	io.enable();

	/*
		The NAND interface can cause three possible interrupts!
	*/
	pic.enable(&disk, VECTOR_IRQ_GPMI);
	pic.enable(&disk, VECTOR_IRQ_GPMI_DMA);
	pic.enable(&disk, VECTOR_IRQ_BCH);
	disk.enable();



	{
	/*
		Remove this block code.  Move the reset() into the disk.enable().
	*/
	io.hex();
	disk.reset();		// FIX THIS
//	uint8_t status = disk.status();
	uint8_t buffer[4096 + 224];
	disk.read_sector(buffer, 123);

	for (int x = 0; x < 4096; x++)
		io << buffer[x] << " ";
	}

#else
	cpu.enable_IRQ();
	pic.enable(&timer, 0x04);
	timer.enable();
	pic.enable(&io, 0x0C);
	io.enable();
	pic.enable(&keyboard, 0x1F, 0x03);
	keyboard.enable();
	pic.enable(&mouse, 0x1F, 0x04);
	mouse.enable();
#endif
}

/*
	ATOSE::GET_GLOBAL_ENTRY_POINT()
	-------------------------------
*/
ATOSE *ATOSE::get_global_entry_point(void)
{
return ATOSE_addr;
}
