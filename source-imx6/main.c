/*
	MAIN.C
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This module contains the main entry point, it also houses (on its stack) the
	object that is the operating system
*/
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsepit.h"

#include "debug_kernel.h"
#include "atose.h"

#define DEFAULT_TIMER 1

/*
	DELAY_INIT()
	------------
*/
void delay_init(void)
{
uint32_t speed_in_Hz[] = {528000000, 396000000, 352000000, 198000000, 594000000};
uint32_t frequency;

HW_CCM_CCGR1.B.CG6 = 0x03;

HW_EPIT_CR_WR(DEFAULT_TIMER, BM_EPIT_CR_SWR);
while ((HW_EPIT_CR(DEFAULT_TIMER).B.SWR) != 0)
	;	// nothing

frequency = speed_in_Hz[HW_CCM_CBCMR.B.PRE_PERIPH_CLK_SEL] / (HW_CCM_CBCDR.B.AHB_PODF + 1) / (HW_CCM_CBCDR.B.IPG_PODF + 1);
HW_EPIT_CR_WR(DEFAULT_TIMER, BF_EPIT_CR_CLKSRC(1) | BF_EPIT_CR_PRESCALAR((frequency / 1000000) - 1) | BM_EPIT_CR_RLD | BM_EPIT_CR_IOVW | BM_EPIT_CR_ENMOD);
}

/*
	DELAY_US()
	----------
*/
void delay_us(uint32_t time_in_us)
{
HW_EPIT_LR_WR(DEFAULT_TIMER, time_in_us);
HW_EPIT_SR_SET(DEFAULT_TIMER, BM_EPIT_SR_OCIF);
HW_EPIT_CR_SET(DEFAULT_TIMER, BM_EPIT_CR_EN);

while (HW_EPIT_SR_RD(DEFAULT_TIMER) == 0)
	;	// nothing (i.e. wait)

HW_EPIT_CR_CLR(DEFAULT_TIMER, BM_EPIT_CR_EN);
}

/*
	CONFIGURE_SABERLITE()
	---------------------
	This is the wealth of stuff that the SABRE Lite does before starting u-boot. Most of it we can ignore
	because we configure it ourselves later, but in the mean time we need to set up the RAM exactly how they
	do it so we just use the same configuration they do.

	Do not remove he delay_us(1) statements because if you do the initialisation happens too quickly and the
	RAM will fail!  Do not remove thre volatile keyword because if you do then the compiler will optimise 
	this incorrectly/

	This code is a post-processing of the Freescale DDR configuration file from u-boot.  The original can
	be found in u-boot-imx6\board\freescale\imx\ddr\mx6q_4x_mt41j128.cfg (which is not part of the ATOSE
	archives).  The original code contained a copyright notice:
		Copyright (C) 2011 Freescale Semiconductor, Inc.
		Jason Liu <r64343@freescale.com>
	It was licensed under "under the terms of the GNU General Public License as published by the Free 
	Software Foundation; either version 2 of the License or (at your option) any later version."

	I doubt the GPL is parasitic in this case because the code below is the output of a manual process.
*/
void configure_saberlite(void)
{
delay_init();

*(volatile uint32_t *)(0x020e05a8) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e05b0) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e0524) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e051c) = 0x00000030; delay_us(1);

*(volatile uint32_t *)(0x020e0518) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e050c) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e05b8) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e05c0) = 0x00000030; delay_us(1);

*(volatile uint32_t *)(0x020e05ac) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e05b4) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0528) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0520) = 0x00020030; delay_us(1);

*(volatile uint32_t *)(0x020e0514) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0510) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e05bc) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e05c4) = 0x00020030; delay_us(1);

*(volatile uint32_t *)(0x020e056c) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0578) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0588) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0594) = 0x00020030; delay_us(1);

*(volatile uint32_t *)(0x020e057c) = 0x00020030; delay_us(1);
*(volatile uint32_t *)(0x020e0590) = 0x00003000; delay_us(1);
*(volatile uint32_t *)(0x020e0598) = 0x00003000; delay_us(1);
*(volatile uint32_t *)(0x020e058c) = 0x00000000; delay_us(1);

*(volatile uint32_t *)(0x020e059c) = 0x00003030; delay_us(1);
*(volatile uint32_t *)(0x020e05a0) = 0x00003030; delay_us(1);
*(volatile uint32_t *)(0x020e0784) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e0788) = 0x00000030; delay_us(1);

*(volatile uint32_t *)(0x020e0794) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e079c) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e07a0) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e07a4) = 0x00000030; delay_us(1);

*(volatile uint32_t *)(0x020e07a8) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e0748) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e074c) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e0750) = 0x00020000; delay_us(1);

*(volatile uint32_t *)(0x020e0758) = 0x00000000; delay_us(1);
*(volatile uint32_t *)(0x020e0774) = 0x00020000; delay_us(1);
*(volatile uint32_t *)(0x020e078c) = 0x00000030; delay_us(1);
*(volatile uint32_t *)(0x020e0798) = 0x000C0000; delay_us(1);

*(volatile uint32_t *)(0x021b081c) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b0820) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b0824) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b0828) = 0x33333333; delay_us(1);

*(volatile uint32_t *)(0x021b481c) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b4820) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b4824) = 0x33333333; delay_us(1);
*(volatile uint32_t *)(0x021b4828) = 0x33333333; delay_us(1);

*(volatile uint32_t *)(0x021b0018) = 0x00081740; delay_us(1);

*(volatile uint32_t *)(0x021b001c) = 0x00008000; delay_us(1);
*(volatile uint32_t *)(0x021b000c) = 0x555A7975; delay_us(1);
*(volatile uint32_t *)(0x021b0010) = 0xFF538E64; delay_us(1);
*(volatile uint32_t *)(0x021b0014) = 0x01FF00DB; delay_us(1);
*(volatile uint32_t *)(0x021b002c) = 0x000026D2; delay_us(1);

*(volatile uint32_t *)(0x021b0030) = 0x005B0E21; delay_us(1);
*(volatile uint32_t *)(0x021b0008) = 0x09444040; delay_us(1);
*(volatile uint32_t *)(0x021b0004) = 0x00025576; delay_us(1);
*(volatile uint32_t *)(0x021b0040) = 0x00000027; delay_us(1);
*(volatile uint32_t *)(0x021b0000) = 0x831A0000; delay_us(1);

*(volatile uint32_t *)(0x021b001c) = 0x04088032; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x0408803A; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x00008033; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x0000803B; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x00428031; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x00428039; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x09408030; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x09408038; delay_us(1);

*(volatile uint32_t *)(0x021b001c) = 0x04008040; delay_us(1);
*(volatile uint32_t *)(0x021b001c) = 0x04008048; delay_us(1);
*(volatile uint32_t *)(0x021b0800) = 0xA1380003; delay_us(1);
*(volatile uint32_t *)(0x021b4800) = 0xA1380003; delay_us(1);
*(volatile uint32_t *)(0x021b0020) = 0x00005800; delay_us(1);
*(volatile uint32_t *)(0x021b0818) = 0x00022227; delay_us(1);
*(volatile uint32_t *)(0x021b4818) = 0x00022227; delay_us(1);

*(volatile uint32_t *)(0x021b083c) = 0x434B0350; delay_us(1);
*(volatile uint32_t *)(0x021b0840) = 0x034C0359; delay_us(1);
*(volatile uint32_t *)(0x021b483c) = 0x434B0350; delay_us(1);
*(volatile uint32_t *)(0x021b4840) = 0x03650348; delay_us(1);
*(volatile uint32_t *)(0x021b0848) = 0x4436383B; delay_us(1);
*(volatile uint32_t *)(0x021b4848) = 0x39393341; delay_us(1);
*(volatile uint32_t *)(0x021b0850) = 0x35373933; delay_us(1);
*(volatile uint32_t *)(0x021b4850) = 0x48254A36; delay_us(1);

*(volatile uint32_t *)(0x021b080c) = 0x001F001F; delay_us(1);
*(volatile uint32_t *)(0x021b0810) = 0x001F001F; delay_us(1);

*(volatile uint32_t *)(0x021b480c) = 0x00440044; delay_us(1);
*(volatile uint32_t *)(0x021b4810) = 0x00440044; delay_us(1);

*(volatile uint32_t *)(0x021b08b8) = 0x00000800; delay_us(1);
*(volatile uint32_t *)(0x021b48b8) = 0x00000800; delay_us(1);

*(volatile uint32_t *)(0x021b001c) = 0x00000000; delay_us(1);
*(volatile uint32_t *)(0x021b0404) = 0x00011006; delay_us(1);

/*
	set the default clock gate to save power
*/
*(volatile uint32_t *)(0x020c4068) = 0x00C03F3F; delay_us(1);
*(volatile uint32_t *)(0x020c406c) = 0x0030FC03; delay_us(1);
*(volatile uint32_t *)(0x020c4070) = 0x0FFFC000; delay_us(1);
*(volatile uint32_t *)(0x020c4074) = 0x3FF00000; delay_us(1);
*(volatile uint32_t *)(0x020c4078) = 0x00FFF300; delay_us(1);
*(volatile uint32_t *)(0x020c407c) = 0x0F0000C3; delay_us(1);
*(volatile uint32_t *)(0x020c4080) = 0x000003FF; delay_us(1);

/*
	enable AXI cache for VDOA/VPU/IPU
*/
*(volatile uint32_t *)(0x020e0010) = 0xF00000CF; delay_us(1);
/*
	set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7
*/
*(volatile uint32_t *)(0x020e0018) = 0x007F007F; delay_us(1);
*(volatile uint32_t *)(0x020e001c) = 0x007F007F; delay_us(1);
}

/*
	MAIN()
	------
*/
int main(void)
{
configure_saberlite();	// set up the external RAM (and other stuff too)

ATOSE_atose atose;		// create and initialise everything

atose.reset();			// now pass control to the OS (which will never return)
return 0;				// like as if this is ever going to happen!
}

