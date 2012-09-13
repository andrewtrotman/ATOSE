/*
	NAND_IMX233.H
	-------------
*/
#ifndef NAND_IMX233_H_
#define NAND_IMX233_H_

#include "nand.h"
#include "nand_imx233_dma.h"

/*
	class ATOSE_NAND_IMX233
	-----------------------
*/
class ATOSE_nand_imx233 : public ATOSE_nand
{
private:
	static const uint32_t longest_dma_command = 7;		// length of the lonstest DMA command chain

private:
	ATOSE_nand_imx233_dma dma_chain[longest_dma_command];				// the current DMA chain

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
	void twiddle_one_command(uint8_t *command);
	uint8_t twiddle(uint8_t b);
#endif

protected:
	virtual void send_command(uint8_t *command,  ATOSE_lock *lock);
	virtual void read(uint8_t *buffer, uint32_t length, ATOSE_lock *lock);
	virtual void read_ecc_sector(uint8_t *buffer, uint32_t length, uint8_t *metadata_buffer, ATOSE_lock *lock);
	virtual void write_ecc_sector(uint8_t *buffer, uint32_t length, ATOSE_lock *lock);

public:
	ATOSE_nand_imx233() : ATOSE_nand() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;

#endif /* NAND_IMX233_H_ */

