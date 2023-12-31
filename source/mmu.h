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

class ATOSE_address_space;

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

	/*
		Only one domain is used so lets use domain number 0x02 (so that is isn't 0x00 or 0x01 or 0x0F)
	*/
	static const uint32_t domain = 0x02;

private:
	uint32_t page_count;										// the number of pages (is size page_size) we have 

protected:
	ATOSE_mmu_page_list free_list;
	uint32_t *identity_page_table;								// a copy of the identity page table (used by the kernel)

public:
	uint32_t bad_page;											// flags to cuae a fault if used			(Fauly in any space)
	uint32_t user_data_page;									// flags for a data page of a user process  (ReadWriteExecute in user space)
	uint32_t user_code_page;									// flags for a code page of a user process  (ReadOnly in User space)
	uint32_t os_page;											// flags for ATOSE pages in the user space	(Fault in user space ReadWriteExecute in Kernel space)

protected:
	void push(void *location, uint64_t size_in_bytes);
	void assume(uint32_t *page_table);

public:
	ATOSE_mmu() { page_count = 0; }
	void init(void);

	void push(ATOSE_mmu_page *page);
	ATOSE_mmu_page *pull(void);

	void flush_caches(void);
	void assume(ATOSE_address_space *address_space);				// switch to the given address space
	void assume_identity(void);										// switch to the kernel's identity address space
	uint32_t *get_identity_page_table(void) { return identity_page_table; } 	// return a pointer to the identity page table
};

#endif /* MMU_H_ */
