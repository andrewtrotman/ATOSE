/*
	MMU.H
	-----
*/
#ifndef MMU_H_
#define MMU_H_

#include <stdint.h>
#include <stddef.h>
#include "mmu_page_list.h"
#include "kernel_memory_allocator.h"

/*
	class ATOSE_MMU
	---------------
*/
class ATOSE_mmu : public ATOSE_kernel_memory_allocator
{
public:
	/*
		On the 32-bit ARM the page size is 1MB
	*/
	static const uint64_t page_size = (1 * 1024 * 1024);		// 1MB sections
	static const uint32_t pages_in_address_space = 4096;		// 4096 pages of 1MB = 4GB addressable space
	static const size_t highest_address = 0xFFFFFFFF;			// highest possible memory location in virtual space

private:
	uint32_t page_count;		// the number of pages (is size page_size) we have 

protected:
	ATOSE_mmu_page_list free_list;

public:
	uint32_t bad_page;											// flags to cuae a fault if used
	uint32_t user_data_page;									// flags for a data page of a user process
	uint32_t user_code_page;									// flags for a code page of a user process
	uint32_t os_page;											// flags for ATOSE pages in the user space

protected:
	void push(void *location, uint64_t size_in_bytes);

public:
	ATOSE_mmu() { page_count = 0; }

	void push(ATOSE_mmu_page *page);
	ATOSE_mmu_page *pull(void);
};

#endif /* MMU_H_ */
