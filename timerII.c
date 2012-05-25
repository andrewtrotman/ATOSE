/*
	TIMERII.C
	---------
	Verify the system timer interrupt mechanism (this is written for the ARM versatile ab board)
	this version uses custom I/O and custom startup (no CRTL)
*/
#include <stdint.h>
#include "stack.h"
#include "io_serial.h"
#include "io_angel.h"
#include "pic.h"
#include "timer.h"
#include "cpu.h"
#include "keyboard_mouse_interface.h"

ATOSE_IO_angel io;
ATOSE_stack stacks;
ATOSE_timer timer;
ATOSE_pic pic;
ATOSE_cpu cpu;						  
ATOSE_keyboard_mouse_interface kmi;

volatile long *ticks = (long *)((unsigned char *)0x101E2000 + 0x04);
volatile long happened = 0;


/*
	__CS3_ISR_IRQ()
	---------------
*/
extern "C" {
		void __attribute__ ((interrupt)) __cs3_isr_irq()
		{
		pic.timer_enter();
		timer.acknowledge();

		happened++;

		pic.timer_exit();
		}
}

/*
	MAIN()
	------
*/
int main(void)
{
int x;

unsigned char *KMI_base_address = (unsigned char *)0x10006000;
uint32_t *KMI_status_register = (uint32_t *)(KMI_base_address + 0x04);
uint32_t *KMI_data_register = (uint32_t *)(KMI_base_address + 0x08);

cpu.enable_IRQ();
//pic.timer_enable();
//timer.enable();
kmi.enable();
//pic.keyboard_enable();

io.hex();

/*
for (x = 0; x < 100; x++)
	io << "Ticks:" << *ticks << " interrupts:" << happened << ATOSE_IO::eoln;
io << "Done";
*/



*KMI_data_register =  0xFF;		// reset

for (;;)
	io << "KMI status:" << (long) *KMI_status_register <<  " DATA:" << (long)*KMI_data_register << ATOSE_IO::eoln;

return 0;
}


