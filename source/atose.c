/*
	ATOSE.C
	-------
*/
#include "atose.h"

/*
	ATOSE::ATOSE()
	--------------
*/
ATOSE::ATOSE()
{
/*
	Initialise each of the essential core objects
*/
stack.init();
cpu.init();
pic.init();
io.init();
timer.init();
keyboard.init((unsigned char *)0x10006000);
mouse.init((unsigned char *)0x10007000);

/*
	Now join them together to form a working system
*/
cpu.enable_IRQ();
pic.enable(&timer, 0x04);
timer.enable();
pic.enable(&io, 0x0C);
io.enable();
pic.enable(&keyboard, 0x1F, 0x03);
keyboard.enable();
pic.enable(&mouse, 0x1F, 0x04);
mouse.enable();
}
