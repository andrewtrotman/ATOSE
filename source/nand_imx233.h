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
	void twiddle_all_commands(void);
	void twiddle_one_command(uint8_t *command);
	uint8_t twiddle(uint8_t b);			// hack to get around the data bus hardware error on the FourARM
#endif

protected:
	virtual void send_command(uint8_t *command);
	virtual void read(uint8_t *buffer, uint32_t length);

public:
	ATOSE_nand_imx233() : ATOSE_nand() {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;

#endif /* NAND_IMX233_H_ */

