/*
	CPU_ARM.H
	---------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Basic manipulation of the CPU including management of interrupts
*/
#ifndef CPU_ARM_H_
#define CPU_ARM_H_

#include <stdint.h>

/*
	class ATOSE_CPU_ARM
	-------------------
*/
class ATOSE_cpu_arm
{
public:
	/*
		The ARM CPU modes are:
		0b10000 User			(shares the stack with system mode)
		0b10001 FIQ
		0b10010 IRQ
		0b10011 Supervisor
		0b10111 Abort
		0b11011 Undefined
		0b11111 System			(shares the stack with user mode)
	*/
	static const uint32_t MODE_USER = 0x10;
	static const uint32_t MODE_FIRQ = 0x11;
	static const uint32_t MODE_IRQ = 0x12;
	static const uint32_t MODE_SUPERVISOR = 0x13;
	static const uint32_t MODE_ABORT = 0x17;
	static const uint32_t MODE_UNDEFINED = 0x1B;
	static const uint32_t MODE_SYSTEM = 0x1F;
	static const uint32_t MODE_BITS = 0x1F;			// AND with this to get the status from the status register

private:
	uint32_t get_cpsr(void) { uint32_t answer; asm volatile ("mrs %0,CPSR" :"=r" (answer)); return answer; }
	void set_cpsr(uint32_t new_cpsr) { asm volatile("msr CPSR_cxsf, %0"::"r"(new_cpsr)); }

public:
	ATOSE_cpu_arm() {}

	virtual void set_interrupt_handlers(void *address) = 0;
	void enable_irq(void) 						{ asm volatile ("cpsie i":::); }
	void disable_irq(void) 						{ asm volatile ("cpsid i":::); }
	virtual void delay_us(uint32_t time_in_us) = 0;
} ;

#endif
