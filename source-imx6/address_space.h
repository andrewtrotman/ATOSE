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

class ATOSE_mmu;

/*
	class ATOSE_ADDRESS_SPACE
	-------------------------
*/
class ATOSE_address_space
{
public:
	/*
		Permissions for a page in the page table.  Note that these are
		the same as PF_x from the ELF specification
	*/
	enum {NONE = 0, EXECUTE = 1, WRITE = 2, READ = 4};

private:
	ATOSE_mmu *mmu;
	ATOSE_mmu_page_list page_list;
	uint32_t *page_table;
	uint32_t reference_count;

public:
	ATOSE_address_space *next;

protected:
	uint32_t add_page(void *virtual_address, ATOSE_mmu_page *page, uint32_t type);

public:
	ATOSE_address_space() {}
	void initialise(ATOSE_mmu *mmu) { this->mmu = mmu; reference_count = 0; }

	ATOSE_address_space *create(void);
	ATOSE_address_space *create_identity(void);
	uint32_t destroy(void);

	uint8_t *add(void *address, size_t size, uint32_t permissions);
	ATOSE_mmu_page *add_to_identity(void);

	uint32_t *get_page_table(void) { return page_table; }

	uint32_t get_reference_count(void) { return reference_count; }
	ATOSE_address_space *get_reference(void) { reference_count++; return this; }
} ;

#endif
