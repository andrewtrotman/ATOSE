/*
	ATOSE.C
	-------
*/
#include "atose.h"

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

/*
	Now drop into user mode
*/
cpu.enter_user_mode();
}

/*
	ATOSE::GET_GLOBAL_ENTRY_POINT()
	-------------------------------
*/
ATOSE *ATOSE::get_global_entry_point(void)
{
return ATOSE_addr;
}
