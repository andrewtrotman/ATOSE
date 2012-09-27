/*
	NAND.H
	------
*/
#ifndef NAND_H_
#define NAND_H_

#include "device_driver.h"
#include "nand_device.h"
#include "lock.h"

class ATOSE_nand_onfi_parameters;

/*
	class ATOSE_NAND
	----------------
*/
class ATOSE_nand : public ATOSE_device_driver
{
public:
	enum {FAIL = 1, FAILC = 2, R = 4, CSP = 8, VSP = 16, ARDY = 32, RDY = 64, WP_N = 128};
	enum {SUCCEED = 0, SECTOR_CORRUPT = 0xFE, SECTOR_BLANK = 0xFF, INTERFACE_CURRUPT = 0x100};

private:
	static ATOSE_nand_device default_device;

protected:
	ATOSE_lock *lock;
	ATOSE_nand_device current_device;

protected:
	uint32_t nanoseconds_to_ticks(uint32_t nanoseconds, uint32_t frequency_in_mhz);

	/*
		Lowest level commands (enable / send / read / write / etc)
	*/
	virtual void enable(ATOSE_nand_device *device) = 0;
	virtual uint32_t send_command(uint8_t *command, ATOSE_lock *lock = 0) = 0;
	virtual uint32_t wait_for_ready(ATOSE_lock *lock) = 0;
	virtual uint32_t read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock = 0) = 0;
	virtual uint32_t write(uint8_t *buffer, uint32_t length, ATOSE_lock *lock = 0) = 0;
	virtual uint32_t read_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock) = 0;
	virtual uint32_t write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock) = 0;

	/*
		NAND method we don't need to expose
	*/
	uint32_t set_timing_mode(uint32_t mode);

public:
	ATOSE_nand() : ATOSE_device_driver() {}

	/*
		Interface to ATOSE device drivers
	*/
	virtual void enable(void);
	virtual void disable(void) = 0;
	virtual void acknowledge(void) = 0;

	/*
		The interface to the NAND chip
	*/
	uint32_t reset(void);
	uint32_t status(void);
	uint32_t get_parameter_block(ATOSE_nand_onfi_parameters *buffer);
	uint32_t read_sector(uint8_t *destination, uint64_t sector);
	uint32_t write_sector(uint8_t *buffer, uint64_t sector);
	uint32_t erase_block(uint64_t sector);
} ;

extern uint8_t ATOSE_nand_command_reset[];
extern uint8_t ATOSE_nand_command_status[];



#endif /* NAND_H_ */

