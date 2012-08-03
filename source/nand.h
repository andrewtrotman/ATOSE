/*
	NAND.H
	------
*/
#ifndef NAND_H_
#define NAND_H_

#include "device_driver.h"
#include "nand_device.h"

/*
	class ATOSE_NAND
	----------------
*/
class ATOSE_nand : public ATOSE_device_driver
{
protected:
	static ATOSE_nand_device default_device;

protected:
	uint32_t nanoseconds_to_ticks(uint32_t nanoseconds, uint32_t frequency_in_mhz);
	virtual void send_command(uint8_t *command) = 0;
	virtual void read(uint8_t *buffer, uint32_t length) = 0;


public:		// FIX THIS :: make protected
	/*
		The interface to the NAND chip
	*/
	void reset(void);
	uint8_t status(void);


public:
	ATOSE_nand() : ATOSE_device_driver() {}

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;
	virtual void acknowledge(void) = 0;
} ;

extern uint8_t ATOSE_nand_command_reset[];
extern uint8_t ATOSE_nand_command_status[];



#endif /* NAND_H_ */

