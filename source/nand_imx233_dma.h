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
	uint32_t irqoncmplt : 1;
	uint32_t nandlock : 1;
	uint32_t nandwait4ready : 1;
	uint32_t semaphore : 1;
	uint32_t wait4endcmd : 1;
	uint32_t haltonterminate : 1;
	uint32_t terminateflush : 1;
	uint32_t reserved : 2;
	uint32_t pio_words : 4;
	uint32_t bytes : 16;

	void *address;				// send / recieve buffer

	uint32_t pio[15];			// there can be no more than 15 PIO words tops
} ;



#endif /* NAND_IMX233_DMA_H_ */
