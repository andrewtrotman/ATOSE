/*
	ATOSE.H
	-------
*/
#ifndef ATOSE_H_
#define ATOSE_H_

#include "cpu.h"
#include "stack.h"
#include "kernel_memory_allocator.h"
#include "mmu_v5.h"

#include "io_serial.h"
#include "io_angel.h"
#include "io_debug_imx233.h"

#include "pic_pl190.h"
#include "pic_imx233.h"

#include "timer_sp804.h"
#include "timer_imx233.h"

#include "keyboard_mouse_interface.h"

#include "nand.h"
#include "nand_verify.h"
#include "nand_imx233.h"

#include "process_manager.h"

/*
	class ATOSE
	-----------
*/
class ATOSE
{
public:
	ATOSE_cpu cpu;
	ATOSE_stack stack;
//	ATOSE_kernel_memory_allocator heap;		// we no longer use this, we use the mmu object instead
	ATOSE_mmu_v5 heap;
	ATOSE_process_manager process_manager;

#ifdef IMX233

	ATOSE_pic_imx233 pic;
	ATOSE_IO_debug_imx233 io;
	ATOSE_timer_imx233 timer;
	ATOSE_nand_verify<ATOSE_nand_imx233> disk;

#else

	ATOSE_pic_pl190 pic;
	ATOSE_IO_serial io;
	ATOSE_timer_sp804 timer;

	ATOSE_keyboard_mouse_interface keyboard;
	ATOSE_keyboard_mouse_interface mouse;

#endif

public:
	ATOSE();
	static ATOSE *get_global_entry_point();

	void enable(void);
} ;

#endif /* ATOSE_H_ */
