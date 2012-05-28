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

ATOSE_IO_serial io;
ATOSE_stack stacks;
ATOSE_timer timer;
ATOSE_pic pic;
ATOSE_cpu cpu;						  
//ATOSE_keyboard_mouse_interface kmi;

volatile long *ticks = (long *)((unsigned char *)0x101E2000 + 0x04);
volatile long happened = 0;
volatile long what_happened = 0;


/*
	__CS3_ISR_IRQ()
	---------------
*/
extern "C" {
		void __attribute__ ((interrupt ("IRQ"))) __cs3_isr_irq()
		{
//		pic.timer_enter();
		what_happened = *((uint32_t *)0x10140030);

		switch (what_happened)
			{
			case 0:
				break;
			case 1:
				timer.acknowledge();
				break;
			case 2:
				io.acknowledge();
				break;
			default:
				break;
			}

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
pic.timer_enable();
timer.enable();
io.enable();
//kmi.enable();
//pic.keyboard_enable();

io.hex();

for (x = 0; x < 1000; x++)
	io << "Ticks:" << *ticks << " interrupts:" << happened << " what happened:" << what_happened << ATOSE_IO::eoln;
io << "Done";

for (int ch = 0; ch < 10; ch++)
	{
	char got;

	if (io.read_byte(&got))
		io << "got: " << got << ATOSE_IO::eoln;
	}

/*

*KMI_data_register =  0xFF;		// reset

for (;;)
	io << "KMI status:" << (long) *KMI_status_register <<  " DATA:" << (long)*KMI_data_register << ATOSE_IO::eoln;
*/

return 0;
}


