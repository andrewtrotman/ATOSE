/*
	ADDRESS_SPACE.H
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef ADDRESS_SPACE_H_
#define ADDRESS_SPACE_H_

#include <stdint.h>
#include <stddef.h>
#include "mmu_page_list.h"
#include "pipe.h"

class ATOSE_mmu;

/*
	class ATOSE_ADDRESS_SPACE
	-------------------------
*/
class ATOSE_address_space
{
friend class ATOSE_mmu;

public:
	/*
		Permissions for a page in the page table.  Note that these are
		the same as PF_x from the ELF specification
	*/
	enum {NONE = 0, EXECUTE = 1, WRITE = 2, READ = 4};

private:
	ATOSE_mmu *mmu;
	ATOSE_mmu_page_list page_list;
	uint32_t *page_table;			// the physical address of the page table
	uint32_t *soft_page_table;		// the virtual address of the page table
	uint32_t thread_count;			// the number of threads that can see this address space
	uint8_t address_space_id;		// Each address space has a unique ID for TBL tracking purposes.
	
public:
	ATOSE_address_space *next;
	uint8_t *the_heap_break;
	uint8_t *the_stack_break;
	ATOSE_pipe *open_pipes;			// a list of the currently open pipes the address space holds

protected:
	uint32_t add_page(void *virtual_address, ATOSE_mmu_page *page, uint32_t type);
	uint32_t *get_page_table(void) { return page_table; }

public:
	ATOSE_address_space() {}
	void initialise(ATOSE_mmu *mmu, uint8_t address_space_id) { this->mmu = mmu; thread_count = 0; this->address_space_id = address_space_id; }

	ATOSE_address_space *create(void);
	ATOSE_address_space *get_reference(void) { thread_count++; return this; }
	uint32_t destroy(void);

	uint8_t *add(void *address, size_t size, uint32_t permissions);

	void *set_heap_break(uint32_t bytes_to_add, uint32_t permissions);
	void *physical_address_of(void *user_address) { return (void *)((soft_page_table[((uint32_t)user_address) >> 20] & 0xFFF00000) | (((uint32_t)user_address) & 0xFFFFF)); }
} ;

#endif
