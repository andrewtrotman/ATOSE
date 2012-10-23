/*
	ATOSE.H
	-------
*/
#ifndef ATOSE_H_
#define ATOSE_H_

#include "cpu.h"
#include "stack.h"
#include "kernel_memory_allocator.h"
#include "mmu_imx233.h"

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
#include "schedule.h"

/*
	class ATOSE
	-----------
*/
class ATOSE
{
public:
	ATOSE_cpu cpu;
	ATOSE_stack stack;
	ATOSE_process_manager process_manager;
	ATOSE_schedule scheduler;

#ifdef IMX233

	ATOSE_pic_imx233 pic;
	ATOSE_IO_debug_imx233 io;
	ATOSE_timer_imx233 timer;
	ATOSE_nand_verify<ATOSE_nand_imx233> disk;
	ATOSE_mmu_imx233 heap;

#elif defined(QEMU)

	ATOSE_pic_pl190 pic;
	ATOSE_IO_angel io;
	ATOSE_timer_sp804 timer;

	ATOSE_mmu_imx233 heap;

	ATOSE_keyboard_mouse_interface keyboard;
	ATOSE_keyboard_mouse_interface mouse;

#else

	ATOSE_kernel_memory_allocator heap;		// we can assume a MMU for ARM V5 and later, but we might wish to support having no off-chip RAM

#endif

public:
	ATOSE();
	static ATOSE *get_global_entry_point();

	void enable(int (*start)(void));
} ;

#endif /* ATOSE_H_ */
