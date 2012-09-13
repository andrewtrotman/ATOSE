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
protected:
	static ATOSE_nand_device default_device;
	static const uint32_t pages_per_block = 128;				// FIX (get this from the geometory of the device)
//	static const uint32_t bytes_per_sector = 4096;				// FIX (get this from the geometory of the device)
	static const uint32_t bytes_per_page = (4096 + 224);		// FIX (get this from the geometory of the device)
	static const uint32_t bytes_of_metadata_per_sector = 224;	// FIX (get this from the geometory of the device)
	static const uint32_t sectors_per_page = 8;					// FIX (get this from the geometory of the device)

protected:
	ATOSE_lock *lock;

protected:
	uint32_t nanoseconds_to_ticks(uint32_t nanoseconds, uint32_t frequency_in_mhz);

	/*
		Lowest level commands (send / read / write)
	*/
	virtual void send_command(uint8_t *command, ATOSE_lock *lock = 0) = 0;
	virtual void read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock = 0) = 0;
	virtual void read_ecc_sector(uint8_t *buffer, uint32_t length, uint8_t *metadata_buffer, ATOSE_lock *lock) = 0;
	virtual void write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock) = 0;

public:		// FIX THIS :: make protected
	/*
		The interface to the NAND chip
	*/
	void reset(void);
	uint8_t status(void);
	void read_sector(uint8_t *destination, uint64_t sector);
	void write_sector(uint8_t *buffer, uint64_t sector);


public:
	ATOSE_nand() : ATOSE_device_driver() { lock = 0; }

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;
	virtual void acknowledge(void) = 0;
} ;

extern uint8_t ATOSE_nand_command_reset[];
extern uint8_t ATOSE_nand_command_status[];



#endif /* NAND_H_ */

