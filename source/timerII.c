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
ATOSE_keyboard_mouse_interface keyboard((unsigned char *)0x10006000);
ATOSE_keyboard_mouse_interface mouse((unsigned char *)0x10007000);

volatile long *ticks = (long *)((unsigned char *)0x101E2000 + 0x04);

extern volatile uint32_t number_of_ticks;

/*
	MAIN()
	------
*/
int main(void)
{
int x;

cpu.enable_IRQ();
pic.enable(&timer, 0x04);
timer.enable();
pic.enable(&io, 0x0C);
io.enable();
pic.enable(&keyboard, 0x1F, 0x03);
keyboard.enable();
pic.enable(&mouse, 0x1F, 0x04);
mouse.enable();

io.hex();

keyboard.write_byte(0xFF);
mouse.write_byte(0xFF);
mouse.write_byte(0xF4);
for (;;)
	{
	char got;

	if (keyboard.read_byte(&got))
		io << "KBM: " << (long)got << ATOSE_IO::eoln;
	if (mouse.read_byte(&got))
		io << "MOU: " << (long)got << ATOSE_IO::eoln;
	if (io.read_byte(&got))
		io << "COM: " << (long)got << ATOSE_IO::eoln;
	}

return 0;
}


