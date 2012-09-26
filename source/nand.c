/*
	NAND.C
	------
*/
#include <stdint.h>
#include "nand.h"
#include "nand_onfi_parameters.h"
#include "spin_lock.h"
#include "timer_imx233.h"		// remove this

/*
	These are the various NAND commands according to ONFI (http://onfi.org/)
	the format is: length(in bytes), then the command string.  Some commands are broken
	into two parts because the single NAND command requires two transmissions with the
	command line high before it happens.
*/
uint8_t ATOSE_nand_command_reset[] = {0x01, 0xFF};
uint8_t ATOSE_nand_command_status[] = {0x01, 0x70};
uint8_t ATOSE_nand_command_read_parameter_page[] = {0x02, 0xEC, 0x00 };
uint8_t ATOSE_nand_command_read[] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t ATOSE_nand_command_read_end[] = {0x01, 0x30};
uint8_t ATOSE_nand_command_write[] = {0x06, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t ATOSE_nand_command_write_end[] = {0x01, 0x10};
uint8_t ATOSE_nand_command_erase_block[] = {0x04, 0x60, 0x00, 0x00, 0x00};
uint8_t ATOSE_nand_command_erase_block_end[] = {0x01, 0xD0};

/*
	We have a default NAND device which is set to async mode 0 (10 MHz)
*/
/*
	ATOSE_NAND::DEFAULT_DEVICE
	--------------------------
*/
ATOSE_nand_device ATOSE_nand::default_device =
{
/*
	Timing characteristics (the default here is ONFI Mode 0)
*/
10,		// MHz clock
25,		// nanoseconds for address setup
60,		// nanoseconds for data hold
80,		// nanoseconds for data setup

/*
	layout characteristics (the default here is for the FourARM Flash NAND, Micron MT29F8G08ABABAWP)
*/
4096,		// bytes per sector
224,		// metadata bytes per sector
128,		// sectors per block (sectors per erase block)
16			// preferred ECC level
};

/*
	ATOSE_NAND::NANOSECONDS_TO_TICKS()
	----------------------------------
*/
uint32_t ATOSE_nand::nanoseconds_to_ticks(uint32_t nanoseconds, uint32_t frequency_in_mhz)
{
uint32_t period;

period = 1000 / frequency_in_mhz;

return (nanoseconds + period - 1) / period;
}

/*
	ATOSE_NAND::RESET()
	-------------------
	Return 0 on success
*/
uint32_t ATOSE_nand::reset(void)
{
ATOSE_spin_lock lock;
uint32_t current_status;
uint8_t command;

/*
	Send the reset command
	The NAND will never respond to the CPU on a reset so we have to wait 1us for it to respond
*/
send_command(ATOSE_nand_command_reset, lock.clear());
ATOSE_timer_imx233::delay_us(1);							// FIX - put the timer somewhere appropriate

/*
	Wait for the status register to go clear
*/
do
	current_status = status();
while ((current_status & 0x60) != 0x60);

return 0;
}

/*
	ATOSE_NAND::STATUS()
	--------------------
	If *we* fail then it returns 0x100 (INTERFACE_CURRUPT), else returns the ONFI status code returned from the Flash NAND:

	Bit Name    Meaning
	7   WP_n   If set to one, then the device is not write protected
	6   RDY    If set to one, then the LUN or plane address is ready for another command and all other bits in the status value are valid
	5   ARDY   If set to one, then there is no array operation in progress
	4   VSP    Vendor Specific
	3   CSP    Command Specific: This bit has command specific meaning
	2   R      Reserved
	1   FAILC  If set to one, then the command issued prior to the last command failed
	0   FAIL   If set to one, then the last command failed
*/
uint32_t ATOSE_nand::status(void)
{
ATOSE_spin_lock lock;
uint8_t answer;

if (send_command(ATOSE_nand_command_status, lock.clear()) == 0)		// succeeds
	if (read(&answer, 1, lock.clear()) == 0)							// succeeds
		return answer;													// success

return INTERFACE_CURRUPT;		// simulate a failure.
}

/*
	ATOSE_NAND::GET_PARAMETER_BLOCK()
	---------------------------------
	Return 0 on success, 0x100 (INTERFACE_CURRUPT) on failure
	any other value is the number of bad parameter blocks we examined before we got a good one
*/
uint32_t ATOSE_nand::get_parameter_block(uint8_t *buffer)
{
ATOSE_spin_lock lock;
uint8_t trial;

/*
	The ONFI spec states that there are at least 3 copies of the 255-byte
	parameter page.  Here we send one command and try reading each one
	until we get one that checksums.  There might be more than 3, but
	unless we get a page that does checksum we cannot determine how many
	there are - so it is pointless checking for more than those initial
	three.
*/
if (send_command(ATOSE_nand_command_read_parameter_page, lock.clear()) == 0)		// success
	for (trial = 0; trial < 3; trial++)
		if (read(buffer, sizeof(ATOSE_nand_onfi_parameters), lock.clear()) == 0)
			{
			/*
				The ONFI spec stats that we only checksum bytes 0..253 inclusive
				This is because bytes 254 and 255 contain the checksum itself.
			*/
			if (ATOSE_nand_onfi_parameters::compute_crc(buffer, 254) == ((ATOSE_nand_onfi_parameters *)buffer)->crc)
				return trial;
			}

return INTERFACE_CURRUPT;
}

/*
	ATOSE_NAND::READ_SECTOR()
	-------------------------
	Returns:
		0x00  (SUCCESS)           success (no errors, no bits corrected)
		0x01 - 0xF0               the number of corrected bits in this sector
		0xFE  (SECTOR_CURRUPT)    data corruption
		0xFF  (SECTOR_BLANK)      sector is blank
		0x100 (INTERFACE_CURRUPT) cannot communicate with the NAND (hardware failure)
*/
uint32_t ATOSE_nand::read_sector(uint8_t *destination, uint64_t sector)
{
uint8_t command[7];
ATOSE_spin_lock lock;

command[0] = ATOSE_nand_command_read[0];
command[1] = ATOSE_nand_command_read[1];
command[2] = 0;							 	// Column number must be 0
command[3] = 0;								// Column number must be 0
command[4] = sector & 0xFF;
command[5] = (sector >> 8) & 0xFF;
command[6] = (sector >> 16) & 0xFF;

if (send_command(command, lock.clear()) == 0)
	if (send_command(ATOSE_nand_command_read_end, lock.clear()) == 0)
		return read_ecc_sector(destination, current_device.bytes_per_sector + current_device.metadata_bytes_per_sector, lock.clear());

return INTERFACE_CURRUPT;
}

/*
	ATOSE_NAND::WRITE_SECTOR()
	--------------------------
	Return 0 on success, 0x100 (INTERFACE_CURRUPT) on failure
*/
uint32_t ATOSE_nand::write_sector(uint8_t *data, uint64_t sector)
{
uint8_t command[7];
ATOSE_spin_lock lock;

command[0] = ATOSE_nand_command_write[0];
command[1] = ATOSE_nand_command_write[1];
command[2] = 0;							 	// Column number must be 0
command[3] = 0;								// Column number must be 0
command[4] = sector & 0xFF;
command[5] = (sector >> 8) & 0xFF;
command[6] = (sector >> 16) & 0xFF;

if (send_command(command, lock.clear()) == 0)
	if (write_ecc_sector(data, current_device.bytes_per_sector + current_device.metadata_bytes_per_sector, lock.clear()) == 0)
		if (send_command(ATOSE_nand_command_write_end, lock.clear()) == 0)
			return 0;

return INTERFACE_CURRUPT;
}

/*
	ATOSE_NAND::ERASE_BLOCK()
	-------------------------
	Return 0 on success, 0x100 (INTERFACE_CURRUPT) on failure
*/
uint32_t ATOSE_nand::erase_block(uint64_t sector)
{
ATOSE_spin_lock lock;
uint8_t command[5];
uint32_t block = sector / current_device.sectors_per_block;

command[0] = ATOSE_nand_command_erase_block[0];
command[1] = ATOSE_nand_command_erase_block[1];
command[2] = block & 0xFF;
command[3] = (block >> 8) & 0xFF;
command[4] = (block >> 16) & 0xFF;

if (send_command(command, lock.clear()) == 0)
	if (send_command(ATOSE_nand_command_erase_block_end, lock.clear()) == 0)
		return 0;

return INTERFACE_CURRUPT;
}
