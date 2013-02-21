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

/*
	ATOSE_TIMER_IMX6Q::ATOSE_TIMER_IMX6Q()
   --------------------------------------
*/
ATOSE_timer_imx6q::ATOSE_timer_imx6q() : ATOSE_timer()
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
HW_GPT_PR.B.PRESCALER = 24;	// divide by 24 to get MHz

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
ATOSE_atose *os;
ATOSE_process *current_process, *next_process;

/*
	Service the interrupt
*/
HW_GPT_SR.U = HW_GPT_SR.U;


/*
	Get a handle to the ATOSE object
*/
os = ATOSE_atose::get_ATOSE();

/*
	What's running and what's next to run?
*/
current_process = os->scheduler.get_current_process();
next_process = os->scheduler.get_next_process() ;

/*
	if the current process is the next process then there is no work to do
*/
if (current_process != next_process)
	{
	os->debug << "switch";
	/*
		If we're running a process then copy the registers into its register space
		this way if we cause a context switch then we've not lost anything
	*/
	if (current_process != NULL)
		memcpy(&current_process->execution_path->registers, registers, sizeof(*registers));

	/*
		Context switch
	*/
	if (next_process != NULL)
		{
		/*
			Set the registers so that we fall back to the next context
		*/
		memcpy(registers, &next_process->execution_path->registers, sizeof(*registers));

		/*
			Set the address space to fall back to the next context
		*/
		os->heap.assume(next_process->address_space);
		}
	}
}

/*
	ATOSE_TIMER_IMX6Q::GET_INTERRUP_ID()
	------------------------------------
*/
uint32_t ATOSE_timer_imx6q::get_interrup_id(void)
{
return IMX_INT_GPT;
}
