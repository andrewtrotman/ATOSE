/*
	ATOSE.C
	-------
*/
#include "atose.h"
#include "process.h"

#ifdef IMX233
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"
#endif

extern ATOSE *ATOSE_addr;

/*
	ATOSE::ATOSE()
	--------------
*/
ATOSE::ATOSE() : heap(), scheduler(&heap)
{
/*
	First things first... put a pointer to me at the end of the interrupt space so that
	we can at ant time get a pointer to this object (by calling ATOSE::get_global_entry_point()
*/
ATOSE_addr = this;

/*
	Initialise each of the essential core objects
*/
stack.init();
cpu.init();
pic.init();
io.init();
timer.init();
heap.init();

#ifdef IMX233
	disk.init();
#else
	keyboard.init((unsigned char *)0x10006000);
	mouse.init((unsigned char *)0x10007000);
#endif

/*
	Now join them together to form a working system
*/
cpu.enable_IRQ();

#ifdef IMX233
	pic.enable(&timer, VECTOR_IRQ_RTC_1MSEC);
	timer.enable();

	/*
		Serial (debug) port
	*/
	pic.enable(&io, VECTOR_IRQ_DEBUG_UART);
	io.enable();

	/*
		The NAND interface can cause three possible interrupts!
	*/
	pic.enable(&disk, VECTOR_IRQ_GPMI);
	pic.enable(&disk, VECTOR_IRQ_GPMI_DMA);
	pic.enable(&disk, VECTOR_IRQ_BCH);
	disk.enable();
#else
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

/*
	ATOSE::ENABLE()
	---------------
*/
void ATOSE::enable(int (*start)(void))
{
scheduler.create_idle_process(start);

/*
	drop into user space (so that the scheduler will take over) and then
	we can halt here (because we will never be scheduled
*/
asm volatile
        (
        "mrs r0, CPSR;"
        "bic r0, r0, #0x01F;"
        "orr r0, r0, #0x10;"
        "msr cpsr_cxsf, r0;"
        :
        :
        : "r0"
        );

for (;;);
}
