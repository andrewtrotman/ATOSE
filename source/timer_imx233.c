/*
	TIMER_IMX233.C
	--------------
	Control the 1ms timer on the i.MX233
*/
#include <stdint.h>
#include "timer_imx233.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsrtc.h"

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
volatile uint32_t got;

got = HW_RTC_CTRL_RD();
HW_RTC_CTRL_CLR(BM_RTC_CTRL_ONEMSEC_IRQ);
}
