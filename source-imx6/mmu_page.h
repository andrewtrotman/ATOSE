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
	ATOSE_mmu_page *next;		// pages are stored in a liked list
	uint8_t *physical_address;	// the physical address
	uint32_t page_size;			// size of the page (in bytes)
} ;

#endif /* MMU_PAGE_H_ */
