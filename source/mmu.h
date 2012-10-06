/*
	MMU.H
	-----
*/
#ifndef MMU_H_
#define MMU_H_

#include <stdint.h>
#include "mmu_page_list.h"
#include "kernel_memory_allocator.h"

/*
	class ATOSE_MMU
	---------------
*/
class ATOSE_mmu : public ATOSE_kernel_memory_allocator
{
protected:
	/*
		On the 32-bit ARM the page size is 1MB
	*/
	static const uint64_t page_size = (1 * 1024 * 1024);
	static const uint32_t pages_in_address_space = 4096;		// 4096 pages of 1MB = 4GB addressable space

private:
	uint32_t page_count;		// the number of pages (is size page_size) we have 


protected:
	ATOSE_mmu_page_list free_list;
	ATOSE_mmu_page_list in_use_list;

private:
	void create_kernel_page_table(uint8_t *first_page);


protected:
	void push(void *location, uint64_t size_in_bytes); 

public:
	ATOSE_mmu() { page_count = 0; }
};

#endif /* MMU_H_ */
