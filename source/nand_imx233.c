/*
	NAND_IMX233.C
	-------------
*/


class ATOSE_nand_imx233 : public ATOSE_nand
{
private:
	void enable_pins(void);
	void enable_clock(void);
	void enable_interface(void);

public:
	ATOSE_nand_imx233() : ATOPSE_nand() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;


/*
	ATOSE_NAND_IMX233::ENABLE_PINS()
	--------------------------------
	Enable the pins and set the drive strength so that CPU can talk to the NAND chip.
	Note that the FourARM has only one NAND and so we only enable control lines to chip 0.
*/
void ATOSE_nand_imx233::enable_pins(void)
{
/*
	Turn on the 8-bit data bus between the CPU and the NAND chip
*/
HW_PINCTRL_MUXSEL0_CLR(BM_PINCTRL_MUXSEL0_BANK0_PIN07 | BM_PINCTRL_MUXSEL0_BANK0_PIN06 | BM_PINCTRL_MUXSEL0_BANK0_PIN05 | BM_PINCTRL_MUXSEL0_BANK0_PIN04 | BM_PINCTRL_MUXSEL0_BANK0_PIN03 | BM_PINCTRL_MUXSEL0_BANK0_PIN02 | BM_PINCTRL_MUXSEL0_BANK0_PIN01 | BM_PINCTRL_MUXSEL0_BANK0_PIN00);

/*
	Turn on the control lines between the CPU and the NAND chip
*/
HW_PINCTRL_MUXSEL1_CLR(BM_PINCTRL_MUXSEL1_BANK0_PIN24 | BM_PINCTRL_MUXSEL1_BANK0_PIN23	| BM_PINCTRL_MUXSEL1_BANK0_PIN19 | BM_PINCTRL_MUXSEL1_BANK0_PIN17 | BM_PINCTRL_MUXSEL1_BANK0_PIN16);
HW_PINCTRL_MUXSEL5_CLR(BM_PINCTRL_MUXSEL5_BANK2_PIN28 | BM_PINCTRL_MUXSEL5_BANK2_PIN25);

/*
	Drive the data bus at 4mA
*/
HW_PINCTRL_DRIVE0_CLR(BM_PINCTRL_DRIVE0_BANK0_PIN07_MA | BM_PINCTRL_DRIVE0_BANK0_PIN06_MA | BM_PINCTRL_DRIVE0_BANK0_PIN05_MA | BM_PINCTRL_DRIVE0_BANK0_PIN04_MA | BM_PINCTRL_DRIVE0_BANK0_PIN03_MA | BM_PINCTRL_DRIVE0_BANK0_PIN02_MA | BM_PINCTRL_DRIVE0_BANK0_PIN01_MA | BM_PINCTRL_DRIVE0_BANK0_PIN00_MA);

/*
	Drive WPN at 12mA and other lines in this register at 4mA
*/

HW_PINCTRL_DRIVE2_WR(BF_PINCTRL_DRIVE2_BANK0_PIN23_MA(2) | BF_PINCTRL_DRIVE2_BANK0_PIN19_MA(0) | BF_PINCTRL_DRIVE2_BANK0_PIN17_MA(0) | BF_PINCTRL_DRIVE2_BANK0_PIN16_MA(0));

/*
	Drive RDN and WRN at 12mA
*/
HW_PINCTRL_DRIVE3_WR((HW_PINCTRL_DRIVE3_RD() & ~(BM_PINCTRL_DRIVE3_BANK0_PIN25_MA | BM_PINCTRL_DRIVE3_BANK0_PIN24_MA) | (BF_PINCTRL_DRIVE3_BANK0_PIN25_MA(2) | BF_PINCTRL_DRIVE3_BANK0_PIN24_MA(2)))

/*
	Drive the remaining lines at 4mA
*/
HW_PINCTRL_DRIVE11_CLR(BM_PINCTRL_DRIVE11_BANK2_PIN28_MA);

/*
	Turn off the pull-up resisters and turn on the pad keepers so that signals flow down the bus
	otherwise the pull-up resisters cause all 1s on the bus.  RDY apparently needs a pull-up.
*/
HW_PINCTRL_PULL0_CLR(BM_PINCTRL_PULL0_BANK0_PIN07 | BM_PINCTRL_PULL0_BANK0_PIN06 | BM_PINCTRL_PULL0_BANK0_PIN05 | BM_PINCTRL_PULL0_BANK0_PIN04 | BM_PINCTRL_PULL0_BANK0_PIN03 | BM_PINCTRL_PULL0_BANK0_PIN02 | BM_PINCTRL_PULL0_BANK0_PIN01 | BM_PINCTRL_PULL0_BANK0_PIN00);
HW_PINCTRL_PULL0_SET(BM_PINCTRL_PULL0_BANK0_PIN19);		// pull up on GPMI_RDY0
HW_PINCTRL_PULL2_CLR(BM_PINCTRL_PULL2_BANK2_PIN28 | BM_PINCTRL_PULL2_BANK2_PIN27);
}

/*
	ATOSE_NAND_IMX233::ENABLE_CLOCK()
	---------------------------------
	Enable the GPMI clock at (initially) gpmi_freq MHz
*/
void ATOSE_nand_imx233::enable_clock(void)
{
static const uint32_t gmpi_frew = 10;				// MHz
static const uint32_t gpmi_div = 480 / gpmi_freq;	// CPU frequency (ref_io) divided by the NAND frequency

/*
	set ref_io frequency to 480 * (18 / IOFRAC)
	so we set IOFRAC to 18
*/
HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_IOFRAC);
HW_CLKCTRL_FRAC_SET(BF_CLKCTRL_FRAC_IOFRAC(18));

/*
	now enable ref_io
*/
HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEIO);

/*
	select ref_io as the source to the GPMI
*/
HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_GPMI);

/*
	Wait until the GPMI has transferred this across its domains
*/
while (HW_CLKCTRL_GPMI.B.BUSY);	// do nothing

/*
	set the GPMI frequency
*/
HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_CLKGATE); // CLKGATE must be cleared to set divisor
HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);
HW_CLKCTRL_GPMI.B.DIV = gpmi_div; // Set GPMI clock divider from io_clk

/*
	Wait until the GPMI has transferred this across its domains
*/
while (HW_CLKCTRL_GPMI.B.BUSY);	// do nothing
}


/*
	ATOSE_NAND_IMX233::ENABLE_INTERFACE()
	-------------------------------------
*/
void ATOSE_nand_imx233::enable_interface()
{
/*
	Bring the GPMI out of reset then delay 3 GPMI clocks.
	The example code uses the longest of 1ms or it comes out of reset - we'll do that here
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
delay_us(1000);
while (HW_GPMI_CTRL0.B.SFTRST); 		// do nothing

/*
	Turn on the GPMI
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE); // clear waiting for soft-reset to set

/*
	Soft reset the GPMI and wait until it signals completion
*/
HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);
while (!HW_GPMI_CTRL0.B.CLKGATE);		// do nothing

/*
	Now repeat... bring the GPMI out of reset, delay 1ms
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
delay_us(1000);
while (HW_GPMI_CTRL0.B.SFTRST); 		// do nothing

/*
	Now make sure the GMPI is on
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE); // enter non gated state
while (HW_GPMI_CTRL0.B.CLKGATE);		// do nothing









/*
	Configure the GPMI for the FourARM
*/



/////////////
/*
set up the GPMI for the FourARM
*/
while (HW_GPMI_CTRL0.B.RUN)
/* wait */;	// wait until RUN is zero (i.e. not processing a command)

HW_GPMI_CTRL0_SET(
BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) 	// set 8-bit databus
);

//We must disable ENABLE before programming then re-enable afterwards.
HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DLL_ENABLE);
HW_GPMI_CTRL1_WR(BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY);

// RDN delay = 0xF and polarity = RDY_BUSY is busy on low ready on high
HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_GPMI_MODE);

//Switch to NAND mode and select BCH as our ECC
HW_GPMI_CTRL1_SET(
BM_GPMI_CTRL1_DEV_RESET |
BM_GPMI_CTRL1_BCH_MODE
);

//Now we must wait 64 GPMI cycles before continuing
delay_us(64000);

HW_GPMI_TIMING0_WR(
BF_GPMI_TIMING0_ADDRESS_SETUP(ns_to_gpmi_clocks(25, gpmi_freq))
| BF_GPMI_TIMING0_DATA_HOLD(ns_to_gpmi_clocks(60, gpmi_freq))
| BF_GPMI_TIMING0_DATA_SETUP(ns_to_gpmi_clocks(80, gpmi_freq))
);

HW_GPMI_TIMING1_WR(0x00100000); // wait for a looooong time for a timeout
}





/*
	ATOSE_NAND_IMX233::ENABLE()
	---------------------------
*/
void ATOSE_nand_imx233::enable(void)
{
enable_pins();
enable_clock();
enable_interface();
}


