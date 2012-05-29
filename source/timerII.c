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

extern volatile uint32_t number_of_ticks;

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
pic.enable(&timer, 0x04);
timer.enable();
pic.enable(&io, 0x0C);
io.enable();

//pic.timer_enable();
//timer.enable();
//io.enable();
//kmi.enable();
//pic.keyboard_enable();

io.hex();

for (x = 0; x < 500; x++)
	io << "Ticks:" << *ticks << " interrupts:" << (long) number_of_ticks << ATOSE_IO::eoln;
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


