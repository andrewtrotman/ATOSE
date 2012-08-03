/*
	NAND.C
	------
*/
#include <stdint.h>
#include "nand.h"

/*
	These are the various NAND commands according to ONFI (http://onfi.org/)
	the format is: bytes, then the command string
*/
uint8_t ATOSE_nand_command_reset[] = {0x01, 0xFF};
uint8_t ATOSE_nand_command_status[] = {0x01, 0x70};

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
*/
send_command(ATOSE_nand_command_reset);

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
uint8_t answer;

send_command(ATOSE_nand_command_status);
read(&answer, 1);
return answer;
}

