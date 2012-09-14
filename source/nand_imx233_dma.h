/*
	NAND_IMX233_DMA.H
	-----------------
*/
#ifndef NAND_IMX233_DMA_H_
#define NAND_IMX233_DMA_H_

/*
	class ATOSE_NAMD_IMX233_DMA
	---------------------------
*/
class ATOSE_nand_imx233_dma
{
public:
	void *next;			// next member in a chain

	uint32_t command : 2;
	uint32_t chain : 1;
	uint32_t irq : 1;
	uint32_t nand_lock : 1;
	uint32_t nand_wait_4_ready : 1;
	uint32_t dec_sem : 1;
	uint32_t wait4end : 1;
	uint32_t halt_on_terminate : 1;
	uint32_t terminate_flush : 1;
	uint32_t reserved : 2;
	uint32_t pio_words : 4;
	uint32_t bytes : 16;

	void *address;				// send / recieve buffer

	uint32_t pio[15];			// there can be no more than 15 PIO words tops
} ;



#endif /* NAND_IMX233_DMA_H_ */
