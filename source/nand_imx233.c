/*
	NAND_IMX233.C
	-------------
*/
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspinctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsgpmi.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsclkctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbh.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsbch.h"
#include "nand_imx233.h"
#include "timer_imx233.h"

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
HW_PINCTRL_DRIVE3_WR(HW_PINCTRL_DRIVE3_RD() & ~(BM_PINCTRL_DRIVE3_BANK0_PIN25_MA | BM_PINCTRL_DRIVE3_BANK0_PIN24_MA) | (BF_PINCTRL_DRIVE3_BANK0_PIN25_MA(2) | BF_PINCTRL_DRIVE3_BANK0_PIN24_MA(2)));

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
	Enable the GPMI clock at (initially) the frequency needed for the given device
*/
void ATOSE_nand_imx233::enable_clock(ATOSE_nand_device *device)
{
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
HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_CLKGATE); 		// CLKGATE must be cleared to set divisor
HW_CLKCTRL_GPMI_CLR(BM_CLKCTRL_GPMI_DIV_FRAC_EN);
HW_CLKCTRL_GPMI.B.DIV = 480 / device->frequency;	// Set GPMI clock divider from io_clk (device->frequency is in MHz)

/*
	Wait until the GPMI has transferred this across its domains
*/
while (HW_CLKCTRL_GPMI.B.BUSY);	// do nothing
}


/*
	ATOSE_NAND_IMX233::ENABLE_INTERFACE()
	-------------------------------------
*/
void ATOSE_nand_imx233::enable_interface(ATOSE_nand_device *device)
{
/*
	Bring the GPMI out of reset then delay 3 GPMI clocks.
	The example code uses the longest of 1ms or it comes out of reset - we'll do that here
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
ATOSE_timer_imx233::delay_us(1000);
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
ATOSE_timer_imx233::delay_us(1000);
while (HW_GPMI_CTRL0.B.SFTRST); 		// do nothing

/*
	Now make sure the GMPI is on
*/
HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE); // enter non gated state
while (HW_GPMI_CTRL0.B.CLKGATE);		// do nothing


/*
	At this point we're out of reset and can configure the GPMI
*/


/*
	Wait until the last command has finished before re-configuring the GPMI
*/
while (HW_GPMI_CTRL0.B.RUN) ;		// do nothing

/*
	Set data bus width to 8-bit
*/
HW_GPMI_CTRL0_SET(BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT));

/*
	Set:
		"ready on high, busy on low" for the RDY pins
		BCH error correction
		DEV_RESET is incorrectly labled - its actually the write-protect pin!
	by doing a write here we also directly set GPMI_MODE to 0 (i.e. go into NAND mode)
*/
HW_GPMI_CTRL1_WR(BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY | BM_GPMI_CTRL1_DEV_RESET | BM_GPMI_CTRL1_BCH_MODE);

/*
	Program the NAND chip timing parameters into the GPMI
*/
HW_GPMI_TIMING0_WR(BF_GPMI_TIMING0_ADDRESS_SETUP(nanoseconds_to_ticks(device->address_setup, device->frequency)) | BF_GPMI_TIMING0_DATA_HOLD(nanoseconds_to_ticks(device->data_hold, device->frequency)) | BF_GPMI_TIMING0_DATA_SETUP(nanoseconds_to_ticks(device->data_setup, device->frequency)));

/*
	Program the time-out to 4096 GPMI cycles
*/
HW_GPMI_TIMING1.B.DEVICE_BUSY_TIMEOUT = 1;

/*
	Enable IRQ on timeout
*/
HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_TIMEOUT_IRQ_EN);
}

/*
	ATOSE_NAND_IMX233::ENABLE_BCH()
	-------------------------------
*/
void ATOSE_nand_imx233::enable_bch(ATOSE_nand_device *device)
{
/*
	Recall that on the i.MX233 there's a hardware bug in the BCH
	See Errata #2847 "BCH soft reset may cause bus master lock up" (http://cache.freescale.com/files/dsp/doc/errata/IMX23CE.pdf)
	which means we cannot soft-reset the BCH properly.

	The errata says "Do not assert HW_BCH_CTRL_SFTRST".  We do the best we can here.
*/

/*
	Come out of reset
*/

HW_BCH_CTRL_CLR(BM_BCH_CTRL_SFTRST);
ATOSE_timer_imx233::delay_us(1000);				// wait a microsecond (should be 3 BCH clocks)

/*
	Turn on the BCH and wait for it to come up
*/
HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);
while (HW_BCH_CTRL.B.CLKGATE);		// do nothing

/*
	Now program the NAND device's parameters into the BCH
*/
HW_BCH_FLASH0LAYOUT0_WR(BF_BCH_FLASH0LAYOUT0_NBLOCKS(device->blocks_per_page - 1) | BF_BCH_FLASH0LAYOUT0_META_SIZE(device->metadata_size) | BF_BCH_FLASH0LAYOUT0_ECC0(device->ecc_level >> 2) | BF_BCH_FLASH0LAYOUT0_DATA0_SIZE(device->block_size));
HW_BCH_FLASH0LAYOUT1_WR(BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(device->blocks_per_page * device->block_size + device->spare_per_page) | BF_BCH_FLASH0LAYOUT1_ECCN(device->ecc_level >> 2) | BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(device->block_size));

/*
	Enable interrupts
*/
HW_BCH_CTRL_SET(BM_BCH_CTRL_COMPLETE_IRQ_EN);
}

/*
	ATOSE_NAND_IMX233::ENABLE_DMA()
	-------------------------------
*/
void ATOSE_nand_imx233::enable_dma(void)
{
/*
	Take the APBH out of reset
*/
HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_SFTRST);
ATOSE_timer_imx233::delay_us(1000);		// wait 1 ms (should be 3 clocks)

HW_APBH_CTRL0_CLR(BM_APBH_CTRL0_CLKGATE);
while (HW_APBH_CTRL0.B.CLKGATE);			// do nothing

/*
	Now bring up DMA chanel 4
*/
HW_APBH_CTRL0_SET(BF_APBH_CTRL0_RESET_CHANNEL(BV_APBH_CTRL0_RESET_CHANNEL__NAND0));
while (HW_APBH_CTRL0_RD() & BF_APBH_CTRL0_RESET_CHANNEL(BV_APBH_CTRL0_RESET_CHANNEL__NAND0) != 0);	// do nothing

/*
	Unfreeze the chanel
*/
HW_APBH_CTRL0_CLR(BV_APBH_CTRL0_FREEZE_CHANNEL__NAND0);

/*
	Enable interrupts
*/
HW_APBH_CTRL1.B.CH4_CMDCMPLT_IRQ_EN = 1;
}

/*
	ATOSE_NAND_IMX233::ENABLE()
	---------------------------
*/
void ATOSE_nand_imx233::enable(void)
{
#ifdef FourARM
	twiddle_all_commands();
#endif
enable_pins();
enable_clock(&default_device);
enable_interface(&default_device);
enable_bch(&default_device);
enable_dma();
}

/*
	ATOSE_NAND_IMX233::DISABLE()
	----------------------------
*/
void ATOSE_nand_imx233::disable(void)
{
}

/*
	ATOSE_NAND_IMX233::ACKNOWLEDGE()
	--------------------------------
*/
void ATOSE_nand_imx233::acknowledge(void)
{
}

#ifdef FourARM
	/*
		On the FourARM the order of the bits on the data bus is muddled	because the order of 
		the pins coming out of the i.MX233 is not in databus order.  To avoid crossing the
		lines on the mother board we have to cross them in software here.  Hopefully this will
		be fixed in future revisions of the hardware
	*/

	/*
		ATOSE_NAND_IMX233::TWIDDLE_ALL_COMMANDS()
		-----------------------------------------
		twiddle all the commands
	*/
	void ATOSE_nand_imx233::twiddle_all_commands(void)
	{
	twiddle_one_command(ATOSE_nand_command_reset);
	twiddle_one_command(ATOSE_nand_command_status);
	}

	/*
		ATOSE_NAND_IMX233::TWIDDLE_ONE_COMMAND()
		----------------------------------------
		twiddle a single command
	*/
	void ATOSE_nand_imx233::twiddle_one_command(uint8_t *command)
	{
	uint32_t byte;
	for (byte = 1; byte < *command + 1; byte++)
		command[byte] = twiddle(command[byte]);
	}

	/*
		ATOSE_NAND_IMX233::TWIDDLE()
		----------------------------
		twiddle a single byte
	*/
	uint8_t ATOSE_nand_imx233::twiddle(uint8_t b)
	{
	typedef union
		{
		struct
			{
			unsigned char bit0 :1;
			unsigned char bit1 :1;
			unsigned char bit2 :1;
			unsigned char bit3 :1;
			unsigned char bit4 :1;
			unsigned char bit5 :1;
			unsigned char bit6 :1;
			unsigned char bit7 :1;
			} u;
		unsigned char b;
		} input_order;

	typedef union
		{
		struct
			{
			unsigned char bit0 :1;
			unsigned char bit1 :1;
			unsigned char bit2 :1;
			unsigned char bit3 :1;
			unsigned char bit5 :1;
			unsigned char bit4 :1;
			unsigned char bit7 :1;
			unsigned char bit6 :1;
			} u;
		unsigned char b;
		} output_order;

	input_order input;
	output_order output;

	input.b = b;
	output.u.bit0 = input.u.bit0;
	output.u.bit1 = input.u.bit1;
	output.u.bit2 = input.u.bit2;
	output.u.bit3 = input.u.bit3;
	output.u.bit4 = input.u.bit4;
	output.u.bit5 = input.u.bit5;
	output.u.bit6 = input.u.bit6;
	output.u.bit7 = input.u.bit7;

	return output.b;
	}
#endif


/*
	ATOSE_NAND_IMX233::SEND_COMMAND()
	---------------------------------
*/
void ATOSE_nand_imx233::send_command(uint8_t *command)
{
ATOSE_nand_imx233_dma *request = dma_chain;

/*
	Set up the DMA request
*/
request->next = 0;
request->command = BV_APBH_CHn_CMD_COMMAND__DMA_READ;
request->chain = 1;
request->irq = 1;
request->nand_lock = 0;
request->nand_wait_4_ready = 0;
request->dec_sem = 1;
request->wait4end = 1;
request->halt_on_terminate = 0;
request->terminate_flush = 0;
request->pio_words = 1;
request->bytes = *command;
request->address = command + 1;

request->pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_CLE) | BM_GPMI_CTRL0_ADDRESS_INCREMENT | BF_GPMI_CTRL0_XFER_COUNT(*command);

/*
	Give the controller the address of the request
*/
HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(request));

/*
	Now tell the controller to issue the request
*/
HW_APBH_CHn_SEMA_WR(4, 1);

/*
	FIX THIS
*/
ATOSE_timer_imx233::delay_us(100000);
}

/*
	ATOSE_NAND_IMX233::READ()
	-------------------------
*/
void ATOSE_nand_imx233::read(uint8_t *buffer, uint32_t length)
{
ATOSE_nand_imx233_dma *request = dma_chain;

/*
	Set up the DMA request
*/
request->next = 0;
request->command = BV_APBH_CHn_CMD_COMMAND__DMA_WRITE;
request->chain = 0;
request->irq = 1;
request->nand_lock = 0;
request->nand_wait_4_ready = 0;
request->dec_sem = 1;
request->wait4end = 1;
request->halt_on_terminate = 0;
request->terminate_flush = 0;
request->pio_words = 1;
request->bytes = length;
request->address = buffer;

request->pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__READ) | BM_GPMI_CTRL0_WORD_LENGTH |BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(length);
/*
	Give the controller the address of the request
*/
HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(request)); // the address of the DMA request

/*
	Now tell the controller to issue the request
*/
HW_APBH_CHn_SEMA_WR(4, 1); // tell the DMA controller to issue the request

/*
		FIX THIS
*/
ATOSE_timer_imx233::delay_us(100000);

for (uint8_t i = 0; i < length; i++)
	buffer[i] = twiddle(buffer[i]);
}


