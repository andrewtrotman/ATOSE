/*
	MMU_V5.H
	--------
*/
#ifndef MMU_V5_H_
#define MMU_V5_H_

#include "mmu.h"

/*
	class ATOSE_MMU_V5
	------------------
*/
class ATOSE_mmu_v5 : public ATOSE_mmu
{
private:
	/*
		Only one domain is used so lets use domain number 0x02 (so that is isn't 0x00 or 0x01 or 0x0F)
	*/
	static const uint32_t domain = 0x02;
	
public:
	ATOSE_mmu_v5();
	void flush_caches(void);
} ;

#endif /* MMU_V5_H_ */
