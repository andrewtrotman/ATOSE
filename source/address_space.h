/*
	ADDRESS_SPACE.H
	---------------
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
	uint32_t process_id;

protected:
	uint32_t add_page(void *virtual_address, ATOSE_mmu_page *page, uint32_t type);

public:
	ATOSE_address_space(ATOSE_mmu *mmu) { this->mmu = mmu; }

	ATOSE_address_space *create(void);
	ATOSE_address_space *create_identity(void);
	uint32_t destroy(void);

	uint8_t *add(void *address, size_t size, uint32_t permissions);
	uint32_t *get_page_table(void) { return page_table; }
} ;


#endif /* ADDRESS_SPACE_H_ */
