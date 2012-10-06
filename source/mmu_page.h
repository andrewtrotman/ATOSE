/*
	MMU_PAGE.H
	----------
*/
#ifndef MMU_PAGE_H_
#define MMU_PAGE_H_

#include <stdint.h>

/*
	class ATOSE_MMU_PAGE
	--------------------
*/
class ATOSE_mmu_page
{
public:
	ATOSE_mmu_page *next;
	uint8_t *physical_address;
	uint32_t process_id;
} ;

#endif /* MMU_PAGE_H_ */
