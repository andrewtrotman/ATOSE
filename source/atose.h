/*
	ATOSE.H
	-------
*/
#ifndef ATOSE_H_
#define ATOSE_H_

#include "stack.h"
#include "io_serial.h"
#include "io_angel.h"
#include "pic.h"
#include "timer.h"
#include "cpu.h"
#include "keyboard_mouse_interface.h"

/*
	class ATOSE
	-----------
*/
class ATOSE
{
public:
	ATOSE_cpu cpu;
	ATOSE_stack stack;
	ATOSE_pic pic;
	ATOSE_IO_serial io;
	ATOSE_timer timer;
	ATOSE_keyboard_mouse_interface keyboard;
	ATOSE_keyboard_mouse_interface mouse;

public:
	ATOSE();

	void enable(void);
} ;

#endif /* ATOSE_H_ */
