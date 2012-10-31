/*
	TIMER_IMX233.C
	--------------
	Control the 1ms timer on the i.MX233
*/
#include <stdint.h>
#include "timer_imx233.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsrtc.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsdigctl.h"

/*
	ATOSE_TIMER_IMX233::ENABLE()
	----------------------------
*/
void ATOSE_timer_imx233::enable(void)
{
HW_RTC_CTRL_SET(BM_RTC_CTRL_ONEMSEC_IRQ_EN);
}

/*
	ATOSE_TIMER_IMX233::DISABLE()
	-----------------------------
*/
void ATOSE_timer_imx233::disable(void)
{
HW_RTC_CTRL_CLR(BM_RTC_CTRL_ONEMSEC_IRQ_EN);
}

/*
	ATOSE_TIMER_IMX233::ACKNOWLEDGE()
	---------------------------------
*/
void ATOSE_timer_imx233::acknowledge(void)
{
HW_RTC_CTRL_RD();
HW_RTC_CTRL_CLR(BM_RTC_CTRL_ONEMSEC_IRQ);
}

/*
	ATOSE_TIMER_IMX233::DELAY_US()
	------------------------------
*/
void ATOSE_timer_imx233::delay_us(uint32_t ticks)
{
uint32_t start = HW_DIGCTL_MICROSECONDS_RD();
uint32_t end = start + ticks;

while ((int32_t) (HW_DIGCTL_MICROSECONDS_RD() - end) < 0);	// do nothing
}
