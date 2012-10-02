/*
	MMU.H
	-----
*/
#ifndef MMU_H_
#define MMU_H_

#include <stdint.h>
#include "mmu_page_list.h"

/*
	class ATOSE_MMU
	---------------
*/
class ATOSE_mmu
{
protected:
	/*
		On the 32-bit ARM the page size is 1MB
	*/
	static const uint64_t page_size = (1 * 1024 * 1024);

private:
	uint32_t page_count;

protected:
	ATOSE_mmu_page_list free_list;
	ATOSE_mmu_page_list in_use_list;

protected:
	void push(void *location, uint64_t size_in_bytes); 

public:
	ATOSE_mmu() { page_count = 0; }
	
} ;


#endif /* MMU_H_ */
