/*
	MMU_IMX6Q.H
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef MMU_IMX6Q_H_
#define MMU_IMX6Q_H_

#include "mmu.h"

/*
	class ATOSE_MMU_IMX6Q
	---------------------
*/
class ATOSE_mmu_imx6q : public ATOSE_mmu
{
public:
	ATOSE_mmu_imx6q() : ATOSE_mmu() {}
	virtual void initialise(void)
		{
		/*
			The SABRE Lite board has 1GB RAM located at 0x10000000
		*/
		push((void *)0x10000000, 1024 * 1024 * 1024);
		ATOSE_mmu::initialise();
		}
};

#endif

