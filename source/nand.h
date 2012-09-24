/*
	NAND.H
	------
*/
#ifndef NAND_H_
#define NAND_H_

#include "device_driver.h"
#include "nand_device.h"
#include "lock.h"

/*
	class ATOSE_NAND
	----------------
*/
class ATOSE_nand : public ATOSE_device_driver
{
public:
	enum {FAIL = 1, FAILC = 2, R = 4, CSP = 8, VSP = 16, ARDY = 32, RDY = 64, WP_N = 128};

protected:
	static ATOSE_nand_device default_device;
	static const uint32_t sectors_per_erase_block = 128;		// FIX (get this from the geometory of the device)
	static const uint32_t bytes_per_sector = (4096 + 224);		// FIX (get this from the geometory of the device)
	static const uint32_t bytes_of_metadata_per_sector = 224;	// FIX (get this from the geometory of the device)
	static const uint32_t subsectors_per_sector = 8;			// FIX (get this from the geometory of the device)

protected:
	ATOSE_lock *lock;

protected:
	uint32_t nanoseconds_to_ticks(uint32_t nanoseconds, uint32_t frequency_in_mhz);

	/*
		Lowest level commands (send / read / write)
	*/
	virtual uint32_t send_command(uint8_t *command, ATOSE_lock *lock = 0) = 0;
	virtual uint32_t read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock = 0) = 0;
	virtual uint32_t read_ecc_sector(uint8_t *buffer, uint32_t length, uint8_t *metadata_buffer, ATOSE_lock *lock) = 0;
	virtual uint32_t write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock) = 0;

public:
	/*
		The interface to the NAND chip
	*/
	uint32_t reset(void);
	uint32_t status(void);
	uint32_t get_parameter_block(uint8_t *buffer);
	uint32_t read_sector(uint8_t *destination, uint64_t sector);
	uint32_t write_sector(uint8_t *buffer, uint64_t sector);
	uint32_t erase_block(uint64_t sector);


public:
	ATOSE_nand() : ATOSE_device_driver() { lock = 0; }

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;
	virtual void acknowledge(void) = 0;
} ;

extern uint8_t ATOSE_nand_command_reset[];
extern uint8_t ATOSE_nand_command_status[];



#endif /* NAND_H_ */

