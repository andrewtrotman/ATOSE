/*
	ATOSE.C
	-------
	Entry point to ATOSE
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


extern "C"
{
	/*
		C_ENTRY()
		---------
	*/
	void c_entry() 
	{
	/*
		to start with we'll make sure the interrupt vectors are correct
	*/
	extern uint32_t ATOSE_vectors_start;
	extern uint32_t ATOSE_vectors_end;
	uint32_t *vectors_src = &ATOSE_vectors_start;
	uint32_t *vectors_dst = (uint32_t *)0;

	while(vectors_src < &ATOSE_vectors_end)
		*vectors_dst++ = *vectors_src++;
	/*
		then we'll call all the constructors
	*/

	extern void (*start_ctors)();
	extern void (*end_ctors)();
	void (**constructor)() = &start_ctors;

	while (constructor < &end_ctors)
		{
		(*constructor)();
		constructor++;
		}

	/*
		Now enter main
	*/
	extern int main(void);
	main();
	}

	/*
		Stubs necessary to get the compiler to work
	*/
	/*
		ABORT()
		-------
	*/
	void abort(void)
	{
	}

	/*
		_SBRK_R ()
		----------
	*/
	void *_sbrk_r(int inc)
	{
	return 0;
	}
}

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

extern uint32_t ATOSE_top_of_memory;

if (&ATOSE_top_of_memory == 0)
	io << "Top of Memory:" << "zero" << ATOSE_IO::eoln;
else
	io << "Top of Memory:" << (long)&ATOSE_top_of_memory << ATOSE_IO::eoln;

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
