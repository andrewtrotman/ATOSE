/*
	TIMER_IMX6Q.C
	-------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Code to control the Freescale i.MX6Q General Purpose Timer (GPT)  That timer is discussed in
	Chapter 30 (pages 1443-1466 of "i.MX 6Dual/6Quad Applications Processor Reference Manual, Rev. 0, 11/2012"

	The primary concern here is to provide a reliable timer for interrupting the processor for thread and
	process scheduling.  An accuracy of milliseconds should suffice.
*/
#include <stdint.h>
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsgpt.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"
#include "atose.h"
#include "timer_imx6q.h"

#include "debug_kernel.h"

/*
	ATOSE_TIMER_IMX6Q::INITIALISE()
   -------------------------------
*/
void ATOSE_timer_imx6q::initialise(void)
{
disable();

/*
   Software Reset
*/
HW_GPT_CR.B.SWR = 0;
while (HW_GPT_CR.B.SWR != 0)
   ;/* nothing */

/*
   Set the clock source and its divider
*/
HW_GPT_CR.B.CLKSRC = 7;		// use the 24MHz crystal
HW_GPT_PR.B.PRESCALER = 23;	// divide by 24 (that is, 23 + 1) to get MHz

/*
	Set the counter to "restart" mode in which it wraps back to 0 when it hits the
	trigger value, and then continues counting.
*/
HW_GPT_CR.B.FRR = 0;

/*
	For some reason (please, someone, work this out), the clock is running 1000 times faster than I think
	it should.  That is, the 24MHz clock is divided by 24 (above) and so 1000 ticks should be 1ms, but it
	turns out that one tick is 1ms!
*/
HW_GPT_OCR1.U = TIME_SLICE_IN_MILLISECONDS;
}

/*
	ATOSE_TIMER_IMX6Q::ENABLE()
	---------------------------
*/
void ATOSE_timer_imx6q::enable(void)
{
/*
   Clear the status register
*/
HW_GPT_SR.U = 0;

/*
	Enable
*/
HW_GPT_CR.B.EN = 1;

/*
	Enable Interrupts
*/
HW_GPT_IR.B.OF1IE = 1;
}

/*
	ATOSE_TIMER_IMX6Q::DISABLE()
	----------------------------
*/
void ATOSE_timer_imx6q::disable(void)
{
/*
   Disable the interrupts
*/
HW_GPT_IR.U = 0;

/*
   Disable output pins then disable input capture pins
*/
HW_GPT_CR.B.OM1 = HW_GPT_CR.B.OM2 = HW_GPT_CR.B.OM3 = 0;
HW_GPT_CR.B.IM1 = HW_GPT_CR.B.IM2 = 0;

/*
   Set counter to 0 on disable
*/
HW_GPT_CR.B.ENMOD = 1;

/*
	Disable the GPT subsystem
*/
HW_GPT_CR.B.EN = 0;
}

/*
	ATOSE_TIMER_IMX6Q::ACKNOWLEDGE()
	--------------------------------
*/
void ATOSE_timer_imx6q::acknowledge(ATOSE_registers *registers)
{
/*
	Service the interrupt
*/
debug_print_this("FLAGS (entry):", HW_GPT_SR.U);
HW_GPT_SR_WR(HW_GPT_SR_RD());
debug_print_this("FLAGS:", HW_GPT_SR.U);

/*
	Internally we store the true return address.  To get that from an IRQ we must subtract 4 from the
	link pointer.  But when we return from and IRQ we subtract four from the link pointer to we need to
	add four once we get the new process's registers back out
*/
registers->r14_current -= 4;

/*
	If we're running a process then copy the registers into its register space
	this way if we cause a context switch then we've not lost anything
*/
if (ATOSE_atose::get_ATOSE()->scheduler.get_current_process() != NULL)
	memcpy(&ATOSE_atose::get_ATOSE()->scheduler.get_current_process()->execution_path->registers, registers, sizeof(*registers));

/*
	Cause a context switch
*/
ATOSE_atose::get_ATOSE()->scheduler.context_switch(registers);
registers->r14_current += 4;

debug_print_this("TIMER:", HW_GPT_CNT_RD());
debug_print_this("FLAGS (exit):", HW_GPT_SR.U);
}

/*
	ATOSE_TIMER_IMX6Q::GET_INTERRUP_ID()
	------------------------------------
*/
uint32_t ATOSE_timer_imx6q::get_interrup_id(void)
{
return IMX_INT_GPT;
}
