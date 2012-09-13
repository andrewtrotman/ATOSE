/*
	NAND.C
	------
*/
#include <stdint.h>
#include "nand.h"
#include "spin_lock.h"
#include "timer_imx233.h"		// remove this

/*
	These are the various NAND commands according to ONFI (http://onfi.org/)
	the format is: bytes, then the command string
*/
uint8_t ATOSE_nand_command_reset[] = {0x01, 0xFF};
uint8_t ATOSE_nand_command_status[] = {0x01, 0x70};
uint8_t ATOSE_nane_command_read_parameter_page[] = {0x01, 0xEC, 0x00 };
uint8_t ATOSE_nand_command_read[] = {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30};

/*
	We have a default NAND device which is set to async mode 0 (10 MHz)
*/
/*
	ATOSE_NAND::DEFAULT_DEVICE
	--------------------------
*/
ATOSE_nand_device ATOSE_nand::default_device =
{
10,		// MHz clock
25,		// nanoseconds for address setup
60,		// nanoseconds for data hold
80,		// nanoseconds for data setup
512,		// bytes per block
0,			// metadata bytes per block
8,			// blocks per page
224,		// bytes of ECC per page
16			// ECC level
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
*/
void ATOSE_nand::reset(void)
{
uint8_t command, current_status;

/*
	Send the reset command
	The NAND will never respond to the CPU on a reset so we have to wait 1us for it to respond
*/
send_command(ATOSE_nand_command_reset);
ATOSE_timer_imx233::delay_us(1);				// FIX - put the timer somewhere appropriate

/*
	Wait for the status register to go clear
*/
do
	current_status = status();
while ((current_status & 0x60) != 0x60);
}

/*
	ATOSE_NAND::STATUS()
	--------------------
*/
uint8_t ATOSE_nand::status(void)
{
ATOSE_spin_lock lock;
uint8_t answer;

send_command(ATOSE_nand_command_status, lock.clear());
read(&answer, 1, lock.clear());

return answer;
}

/*
	ATOSE_NAND::READ_SECTOR()
	-------------------------
*/
void ATOSE_nand::read_sector(uint8_t *destination, uint64_t sector)
{
uint16_t column;
uint32_t row;
uint8_t metadata_buffer[bytes_of_metadata_per_sector];
uint8_t command[sizeof(ATOSE_nand_command_read)];
ATOSE_spin_lock lock;

sector_to_address(sector, &column, &row);
command[0] = ATOSE_nand_command_read[0];
command[1] = ATOSE_nand_command_read[1];
command[2] = column & 0xFF;
command[3] = (column >> 8) & 0xFF;
command[4] = row & 0xFF;
command[5] = (row >> 8) & 0xFF;
command[6] = (row >> 16) & 0xFF;
command[7] = ATOSE_nand_command_read[7];

send_command(command, lock.clear());
read_ecc_sector(destination, bytes_per_page, metadata_buffer, lock.clear());
}
