/*
	ATOSE.H
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef ATOSE_H_
#define ATOSE_H_

#include "fat.h"
#include "stack.h"
#include "debug.h"
#include "cpu_arm.h"
#include "host_usb.h"
#include "usb_imx6q.h"
#include "mmu_imx6q.h"
#include "uart_imx6q.h"
#include "timer_imx6q.h"
#include "cpu_arm_imx6q.h"
#include "process_manager.h"
#include "interrupt_arm_gic.h"

class ATOSE_registers;

/*
	class ATOSE_ATOSE
	-----------------
*/
class ATOSE_atose
{
private:
	/*
		NOTE: In C++ objects are constructed in the order in which they are specified in the class
	*/
	ATOSE_stack stack;
	ATOSE_cpu_arm_imx6q imx6q_cpu;
	ATOSE_interrupt_arm_gic imx6q_gic;
	ATOSE_uart_imx6q imx6q_serial_port;
	ATOSE_mmu_imx6q imx6q_heap;
	ATOSE_timer_imx6q imx6q_process_clock;
//	ATOSE_usb_imx6q imx6q_usb;

public:
	ATOSE_host_usb imx6q_host_usb;
	ATOSE_fat file_system;

public:
	/*
		A few references for syntactic purposes (i.e. standard methods to refer to these objects regardless of subclassing)
	*/
	ATOSE_process_allocator process_allocator;
	ATOSE_process_manager scheduler;
public:
	ATOSE_timer &process_clock;
	ATOSE_mmu &heap;
	ATOSE_debug &debug;
	ATOSE_cpu_arm &cpu;
	ATOSE_interrupt &interrupt_controller;
//	ATOSE_usb &usb;

private:
	void set_ATOSE(void) { extern ATOSE_atose *ATOSE_pointer; ATOSE_pointer = this; }

public:
	ATOSE_atose();

	static ATOSE_atose *get_ATOSE(void) { extern ATOSE_atose *ATOSE_pointer; return ATOSE_pointer; }
	static void *physical_address_of(void *user_space_address) { return ATOSE_atose::get_ATOSE()->scheduler.physical_address_of(user_space_address); }

	virtual void reset(ATOSE_registers *registers = NULL);		// we can't normally have the registers on reset.
	virtual void isr_firq(ATOSE_registers *registers);
	virtual void isr_swi(ATOSE_registers *registers);
	virtual void isr_prefetch_abort(ATOSE_registers *registers);
	virtual void isr_data_abort(ATOSE_registers *registers);
	virtual void isr_undefined(ATOSE_registers *registers);
	virtual void isr_reserved(ATOSE_registers *registers);
} ;

#endif /* ATOSE_H_ */

