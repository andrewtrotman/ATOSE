/*
	NAND_IMX233.H
	-------------
*/
#ifndef NAND_IMX233_H_
#define NAND_IMX233_H_

#include "nand.h"
#include "nand_imx233_dma.h"

class ATOSE_nand_device;
class ATOSE_lock;

/*
	class ATOSE_NAND_IMX233
	-----------------------
*/
class ATOSE_nand_imx233 : public ATOSE_nand
{
private:
	void enable_pins(void);
	void enable_clock(ATOSE_nand_device *device);
	void enable_interface(ATOSE_nand_device *device);
	void enable_bch(ATOSE_nand_device *device);
	void enable_dma(void);

#ifdef FourARM
	/*
		These methods are necessary because of crossed data lines on the FourARM board
		They cross the data in the command so that when it gets to the Flash its correct
	*/
	void twiddle(uint8_t *buffer, uint32_t length);
	void twiddle_one_command(uint8_t *command) { twiddle(command + 1, *command); }
	uint8_t twiddle(uint8_t b);
#endif

protected:
	uint32_t transmit(ATOSE_nand_imx233_dma *request, ATOSE_lock *lock, uint8_t *command = 0);
	virtual uint32_t send_command(uint8_t *command,  ATOSE_lock *lock);
	virtual uint32_t read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock);
	virtual uint32_t read_ecc_sector(uint8_t *buffer, uint32_t length, uint8_t *metadata_buffer, ATOSE_lock *lock);
	virtual uint32_t write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock);

public:
	ATOSE_nand_imx233() : ATOSE_nand() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;


#endif /* NAND_IMX233_H_ */

