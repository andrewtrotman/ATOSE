/*
	MMU_IMX233.H
	------------
*/
#ifndef MMU_IMX233_H_
#define MMU_IMX233_H_

#include "mmu.h"

/*
	class ATOSE_MMU_IMX233
	----------------------
*/
class ATOSE_mmu_imx233 : public ATOSE_mmu
{
public:
	ATOSE_mmu_imx233() : ATOSE_mmu()
	{
	#ifdef FourARM
		/*
			Tell the MMU that the FourARM has 128MB off-chip RAM located at 0x40000000
		*/
		push((void *)0x40000000, 128 * 1024 * 1024);
	#elif defined(QEMU)
		/*
			By default QEMU has 256MB RAM at 0x00000000, but we'll only use the top 255 of that as the bottom 1BM is "reserved" for ATOSE
		*/
		push((void *)(0x00000000 + 1024 * 1024), 255 * 1024 * 1024);
	#endif
	}
};	


#endif /* MMU_IMX233_H_ */
