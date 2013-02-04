/*
	CLOCK_IMX6Q.C
	-------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Code to control the Freescale i.MX6Q Enhanced Periodic Interrupt Timer (EPIT)  That timer is discussed in
	Chapter 24 (pages 1165-1178 of "i.MX 6Dual/6Quad Applications Processor Reference Manual, Rev. 0, 11/2012"
	The primary concern here is to provide a u-second scale timer.
*/
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsepit.h"
#include "clock_imx6q.h"

/*
	ATOSE_CLOCK_IMX6Q::ATOSE_CLOCK_IMX6Q()
	--------------------------------------
*/
void ATOSE_clock_imx6q::ATOSE_clock_imx6q(void)
{
uint32_t speed_in_Hz[] = {528000000, 396000000, 352000000, 198000000, 594000000};
uint32_t frequency;

/*
	Gate the clock line
*/
HW_CCM_CCGR1.B.CG6 = 0x03;

/*
	Software reset the subsystem
*/
HW_EPIT_CR_WR(us_clock, BM_EPIT_CR_SWR);
while ((HW_EPIT_CR(us_clock).B.SWR) != 0)
	;	// nothing

/*
	Configure the timer:
		Use the peripheral clock
		Set and forget mode
		Immediate load starting with the value in the load register
*/
frequency = speed_in_Hz[HW_CCM_CBCMR.B.PRE_PERIPH_CLK_SEL] / (HW_CCM_CBCDR.B.AHB_PODF + 1) / (HW_CCM_CBCDR.B.IPG_PODF + 1);
HW_EPIT_CR_WR(us_clock, BF_EPIT_CR_CLKSRC(1) | BF_EPIT_CR_PRESCALAR((frequency / 1000000) - 1) | BM_EPIT_CR_RLD | BM_EPIT_CR_IOVW | BM_EPIT_CR_ENMOD);
}

/*
	ATOSE_CLOCK_IMX6Q::DELAY_US()
	-----------------------------
*/
void ATOSE_clock_imx6q::delay_us(uint32_t time_in_useconds)
{
/*
	Store the count value in the load register (which then gets immediately loaded into the timer
*/
HW_EPIT_LR_WR(us_clock, time_in_useconds);

/*
	Clear the status register so that we can watch it clock over
*/
HW_EPIT_SR_SET(us_clock, BM_EPIT_SR_OCIF);

/*
	Start the timer
*/
HW_EPIT_CR_SET(us_clock, BM_EPIT_CR_EN);

/*
	Wait for the timer to elapse
*/
while (HW_EPIT_SR_RD(us_clock) == 0)
	;	// nothing (i.e. wait)

/*
	Shut it down
*/
HW_EPIT_CR_CLR(us_clock, BM_EPIT_CR_EN);
}

