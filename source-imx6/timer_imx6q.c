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
#include "timer_imx6q.h"

/*
	ATOSE_TIMER_IMX6Q::ATOSE_TIMER_IMX6Q()
   --------------------------------------
*/
ATOSE_timer_imx6q::ATOSE_timer_imx6q()
{
}

/*
	ATOSE_TIMER_IMX6Q::INIT()
	-------------------------
*/
void ATOSE_timer_imx6q::init(void)
{
disable();

/*
   Set the clock source
*/
HW_GPT_CR.B.CLKSRC = 0x07;      // use the 24MHz crystal

/*
   Software Reset
*/
HW_GPT_CR.B.SWR = 0;
while (HW_GPT_CR.B.SWR != 0)
   ;/* nothing */
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
   Set counter to 0
*/
HW_GPT_CR.B.ENMOD = 1;

/*
	Enable
*/
HW_GPT_CR.B.EN = 1;
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
	Disable the GPT subsystem
*/
HW_GPT_CR.B.EN = 0;
}

/*
	ATOSE_TIMER_IMX6Q::ACKNOWLEDGE()
	--------------------------------
*/
void ATOSE_timer_imx6q::acknowledge(void)
{
/*
	Not sure yet
*/
}

