/*
	NAND_IMX233.C
	-------------
*/
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspinctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsgpmi.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsclkctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbh.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsbch.h"
#include "nand_device.h"
#include "nand_imx233.h"
#include "spin_lock.h"
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
HW_PINCTRL_MUXSEL1_CLR(BM_PINCTRL_MUXSEL1_BANK0_PIN25 | BM_PINCTRL_MUXSEL1_BANK0_PIN24 | BM_PINCTRL_MUXSEL1_BANK0_PIN23	| BM_PINCTRL_MUXSEL1_BANK0_PIN19 | BM_PINCTRL_MUXSEL1_BANK0_PIN17 | BM_PINCTRL_MUXSEL1_BANK0_PIN16);
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
	Turn on the BCH
	Wait for it to come up
*/

HW_BCH_CTRL_CLR(BM_BCH_CTRL_SFTRST);
// ATOSE_timer_imx233::delay_us(1000);	// wait a microsecond (should be 3 BCH clocks), but the i.MX233 manual doesn't do this (see pp15-16)!
HW_BCH_CTRL_CLR(BM_BCH_CTRL_CLKGATE);
while (HW_BCH_CTRL.B.CLKGATE);		// do nothing

/*
	Program the NAND device's parameters into the BCH

	The i.MX233 BCH module "is capable of correcting from 2 to 20 single
	bit errors within a block of data no larger than about 900 bytes (512
	bytes is typical) in applications such as protecting data and
	resources stored on modern NAND flash devices" (see  i.MX23
	Applications Processor Reference Manual IMX23RM, Rev. 1, 11/2009, pp 15-1)

	We must decompose the NAND sectors into smaller blocks of some size
	smaller than "about 900" bytes.  Smaller blocks require more ECC bits
	per sector and so your get fewer useful bits.  Large blocks require
	fewer ECC bits per sector, but have a higher  chance of failure!
	We'll go for a configurable size, but that will, presumably be 512 in
	the normal case.

	
	The BCH allows a meta-data block at the start of the sector.  We don't use it
	and so set metadata size to 0. This makes the first block a "regular" block
	therefore we have n-1 "subsequent" blocks after block 0.
*/
HW_BCH_FLASH0LAYOUT0_WR(BF_BCH_FLASH0LAYOUT0_NBLOCKS(device->bytes_per_sector / bytes_per_BCH_subsector - 1) | BF_BCH_FLASH0LAYOUT0_META_SIZE(0) | BF_BCH_FLASH0LAYOUT0_ECC0(device->ecc_level >> 1) | BF_BCH_FLASH0LAYOUT0_DATA0_SIZE(bytes_per_BCH_subsector));
HW_BCH_FLASH0LAYOUT1_WR(BF_BCH_FLASH0LAYOUT1_PAGE_SIZE(device->bytes_per_sector + device->metadata_bytes_per_sector) | BF_BCH_FLASH0LAYOUT1_ECCN(device->ecc_level >> 1) | BF_BCH_FLASH0LAYOUT1_DATAN_SIZE(bytes_per_BCH_subsector));

/*
	Enable interrupts
	We don't need this because we can get it from the DMA controller
*/
//HW_BCH_CTRL_SET(BM_BCH_CTRL_COMPLETE_IRQ_EN);
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

/*
	Ungate the clock
*/
HW_APBH_CTRL0_CLR(1 << (4 + BP_APBH_CTRL0_CLKGATE_CHANNEL));
}

/*
	ATOSE_NAND_IMX233::ENABLE()
	---------------------------
*/
void ATOSE_nand_imx233::enable(void)
{
enable_pins();
enable_clock(&current_device);
enable_interface(&current_device);
enable_bch(&current_device);
enable_dma();
reset();
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
/*
	Check to see if the GPMI timed out
*/
if (HW_GPMI_CTRL1.B.TIMEOUT_IRQ != 0)
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_TIMEOUT_IRQ);

/*
	Check to see if the GPMI thinks its finished
*/
if (HW_GPMI_CTRL1.B.DEV_IRQ != 0)
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_DEV_IRQ);

/*
	Did the DMA controller think there was an error?
*/
if (HW_APBH_CTRL2.B.CH4_ERROR_STATUS != 0)
	HW_APBH_CTRL2_CLR(BM_APBH_CTRL2_CH4_ERROR_IRQ);

/*
	Tell the DMA that all's well
*/
if (HW_APBH_CTRL1.B.CH4_CMDCMPLT_IRQ != 0)
	HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);

#ifdef NEVER
	/*
		Signal to the BCH error correcton that we're done
		As we've turned off the BCH interrupt, this isn't necessary any more
	*/
	HW_BCH_CTRL_CLR(BM_BCH_CTRL_COMPLETE_IRQ);
#endif

/*
	Signal the lock mechanism to say we're done when the GPMI say's we're done (not when the DMA say's we're done)
*/
if (lock != 0)
	lock->signal();
}

#ifdef FourARM
	/*
		On the FourARM the order of the bits on the data bus is muddled	because the order of 
		the pins coming out of the i.MX233 is not in databus order.  To avoid crossing the
		lines on the mother board we have to cross them in software here.  Hopefully this will
		be fixed in future revisions of the hardware
	*/

	/*
		ATOSE_NAND_IMX233::TWIDDLE_ONE_COMMAND()
		----------------------------------------
		Twiddle the bits in a buffer
	*/
	void ATOSE_nand_imx233::twiddle(uint8_t *buffer, uint32_t length)
	{
	uint8_t *byte, *end;

	end = buffer + length;
	for (byte = buffer; byte < end; byte++)
		*byte = twiddle(*byte);
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
	ATOSE_NAND_IMX233::TRANSMIT()
	-----------------------------
	Return 0 on success, and other result is an error
*/
uint32_t ATOSE_nand_imx233::transmit(ATOSE_nand_imx233_dma *request, ATOSE_lock *lock, uint8_t *command)
{
#ifdef FourARM
	/*
		Re-order the bits for the FourARM because of data-bus swapping
	*/
	if (command != 0)
		twiddle_one_command(command);
#endif

/*
	If the DMA semaphore is non-zero then the DMA is currently executing a command and so we spin.
	Theoretically this should not ever spin if we use spinlocks to wait for the commands to return.
*/
while (HW_APBH_CHn_SEMA(4).B.PHORE != 0);		// do nothing

/*
	Store a handle to the lock
*/
this->lock = lock;

/*
	Give the controller the address of the request
*/
HW_APBH_CHn_NXTCMDAR_WR(4, (uint32_t)(request));

/*
	Now tell the controller to issue the request by setting the semaphore to 1
*/
HW_APBH_CHn_SEMA_WR(4, 1);

/*
	Now we spin waiting for the command to finish
*/
if (lock != 0)
	lock->wait();

this->lock = 0;

#ifdef FourARM
	/*
		Put the command back as it was
	*/
	if (command != 0)
		twiddle_one_command(command);
#endif

return 0;		// no error
}

/*
	ATOSE_NAND_IMX233::SEND_COMMAND()
	---------------------------------
	return 0 on success, and other result is an error
*/
uint32_t ATOSE_nand_imx233::send_command(uint8_t *command,  ATOSE_lock *lock)
{
ATOSE_nand_imx233_dma request;

/*
	Set up the DMA request
*/
request.next = 0;
request.command = BV_APBH_CHn_CMD_COMMAND__DMA_READ;
request.chain = 0;
request.irq = 1;
request.nand_lock = 0;
request.nand_wait_4_ready = 0;
request.dec_sem = 1;
request.wait4end = 1;
request.halt_on_terminate = 0;
request.terminate_flush = 0;
request.pio_words = 3;
request.bytes = *command;
request.address = command + 1;

request.pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_CLE) | BM_GPMI_CTRL0_ADDRESS_INCREMENT | BF_GPMI_CTRL0_XFER_COUNT(*command);
request.pio[1] = 0;
request.pio[2] = 0;	// disable BCH ECC

/*
	Now tell the DMA controller to do the request
*/
return transmit(&request, lock, command);
}

/*
	ATOSE_NAND_IMX233::READ()
	-------------------------
	return 0 on success, and other result is an error
*/
uint32_t ATOSE_nand_imx233::read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock)
{
uint32_t success;
ATOSE_nand_imx233_dma request;

/*
	Set up the DMA request
*/
request.next = 0;
request.command = BV_APBH_CHn_CMD_COMMAND__DMA_WRITE;
request.chain = 0;
request.irq = 1;
request.nand_lock = 0;
request.nand_wait_4_ready = 0;
request.dec_sem = 1;
request.wait4end = 1;
request.halt_on_terminate = 0;
request.terminate_flush = 0;
request.pio_words = 1;
request.bytes = length;
request.address = buffer;

request.pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__READ) | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(length);

/*
	Now tell the DMA controller to do the request
*/
success = transmit(&request, lock);

#ifdef FourARM
	twiddle(buffer, length);
#endif

return success;
}

/*
	ATOSE_NAND_IMX233::READ_ECC_SECTOR()
	------------------------------------
	Returns:
		0x00  (SUCCESS)           success (no errors, no bits corrected)
		0x01 - 0xF0               the number of corrected bits in this sector
		0xFE  (SECTOR_CURRUPT)    data corruption
		0xFF  (SECTOR_BLANK)      sector is blank
		0x100 (INTERFACE_CURRUPT) cannot communicate with the NAND (hardware failure)
*/
uint32_t ATOSE_nand_imx233::read_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock)
{
/*
	Each sub-sector returns an error code and so we must creare space here to recieve them from the BCH
*/
__attribute__((aligned(0x4))) uint8_t status_code[current_device.bytes_per_sector / bytes_per_BCH_subsector];
uint32_t fixed_bits, blank_subsectors, success, current;

/*
	Get to DMA objects and join them together in a chain
*/
ATOSE_nand_imx233_dma request, request2;
request.next = &request2;

/*
	In the first request we put the read (NAND to 1.MX233) request
*/
request.command = BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
request.chain = 1;
request.irq = 0;
request.nand_lock = 0;
request.nand_wait_4_ready = 0;
request.dec_sem = 0;
request.wait4end = 1;
request.halt_on_terminate = 0;
request.terminate_flush = 0;
request.pio_words = 6;
request.bytes = 0;

request.pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__READ) | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(length);
request.pio[1] = 0;
request.pio[2] = BF_GPMI_ECCCTRL_HANDLE(0x123) | BF_GPMI_ECCCTRL_ECC_CMD(BV_GPMI_ECCCTRL_ECC_CMD__DECODE_8_BIT) | BF_GPMI_ECCCTRL_ENABLE_ECC(1) | BF_GPMI_ECCCTRL_BUFFER_MASK(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE);
request.pio[3] = length;
request.pio[4] = (uint32_t)buffer;
request.pio[5] = (uint32_t)status_code;

/*
	In the second request we disable the BCH
*/
request2.command = BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
request2.chain = 0;
request2.irq = 1;
request2.nand_lock = 0;
request2.nand_wait_4_ready = 1;
request2.dec_sem = 1;
request2.wait4end = 1;
request2.halt_on_terminate = 0;
request2.terminate_flush = 0;
request2.pio_words = 3;
request2.bytes = 0;

request2.address = 0;

request2.pio[0] = BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WAIT_FOR_READY) | BM_GPMI_CTRL0_LOCK_CS | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(0);
request2.pio[1] = 0;
request2.pio[2] = 0; // Reset the BCH

/*
	Now tell the DMA controller to do the requests
*/
success = transmit(&request, lock);

#ifdef NEVER
	#ifdef FourARM
		twiddle(buffer, length);
	#endif
#endif

if (success != 0)
	return INTERFACE_CURRUPT;			// cannot talk to the hardware

/*
	Compute the number of bits we corrected in this sector...

	As a consequence of the read the status_codes is loaded with one byte per sub-sector.
	The meanings of the bytes are:

	0x00      No error
	0x01-0x14 Number of corrected errors
	0xFE      Block is corrupt ("uncorrectable")
	0xFF      Block is empty ("Erased")

	So we can go though the array adding up the codes and return the numner of errors
	that were corrected in the sector (or 0xFF for the entire sector is blank, or 0xFE for curruption)
*/
fixed_bits = blank_subsectors = 0;
for (current = 0; current < current_device.bytes_per_sector / bytes_per_BCH_subsector; current++)
	{
	if (status_code[current] == BV_BCH_STATUS0_STATUS_BLK0__UNCORRECTABLE)
		return SECTOR_CORRUPT;
	else if (status_code[current] == BV_BCH_STATUS0_STATUS_BLK0__ERASED)
		blank_subsectors++;
	else
		fixed_bits += status_code[current];
	}

/*
	if the sector is blank then we tell the user this
*/
if (blank_subsectors == current_device.bytes_per_sector / bytes_per_BCH_subsector)
	return SECTOR_BLANK;

/*
	Otherwise we tell them how many bits were corrected in this sector
*/
return fixed_bits;
}

/*
	ATOSE_NAND_IMX233::WRITE_ECC_SECTOR()
	-------------------------------------
	return 0 on success, and other result is an error
*/
uint32_t ATOSE_nand_imx233::write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock)
{
ATOSE_nand_imx233_dma request;
uint32_t success;

/*
	Tell the BCH to do the write
*/
request.command = BV_APBH_CHn_CMD_COMMAND__NO_DMA_XFER;
request.chain = 0;
request.irq = 1;
request.nand_lock = 0;
request.nand_wait_4_ready = 0;
request.dec_sem = 1;
request.wait4end = 1;
request.halt_on_terminate = 0;
request.terminate_flush = 0;
request.pio_words = 6;
request.bytes = 0;

request.address = 0;

request.pio[0] = BM_GPMI_CTRL0_LOCK_CS | BF_GPMI_CTRL0_COMMAND_MODE(BV_GPMI_CTRL0_COMMAND_MODE__WRITE) | BM_GPMI_CTRL0_WORD_LENGTH | BF_GPMI_CTRL0_CS(0) | BF_GPMI_CTRL0_ADDRESS(BV_GPMI_CTRL0_ADDRESS__NAND_DATA) | BF_GPMI_CTRL0_XFER_COUNT(0);
request.pio[1] = 0;
request.pio[2] = BF_GPMI_ECCCTRL_HANDLE(0x321) | BF_GPMI_ECCCTRL_ECC_CMD(BV_GPMI_ECCCTRL_ECC_CMD__ENCODE_8_BIT) | BF_GPMI_ECCCTRL_ENABLE_ECC(1) | BF_GPMI_ECCCTRL_BUFFER_MASK(BV_GPMI_ECCCTRL_BUFFER_MASK__BCH_PAGE);
request.pio[3] = length;
request.pio[4] = (uint32_t)buffer;
request.pio[5] = 0;

#ifdef NEVER
	#ifdef FourARM
		twiddle(buffer, length); 	// Twiddle the data before we write it
	#endif
#endif

/*
	Now tell the DMA controller to do the request
*/
success = transmit(&request, lock);

#ifdef NEVER
	#ifdef FourARM
		twiddle(buffer, length);	// Now return it to how it was before we wrote it
	#endif
#endif

return success;
}
