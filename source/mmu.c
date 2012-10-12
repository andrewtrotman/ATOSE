/*
	MMU.C
	-----
*/
#include "mmu.h"
#include "mmu_page_list.h"
#include "mmu_page.h"
#include "ascii_str.h"

/*
	ATOSE_MMU::PUSH()
	-----------------
*/
void ATOSE_mmu::push(void *location, uint64_t size_in_bytes)
{
uint8_t *page, *end;
ATOSE_mmu_page *current;

end = (uint8_t *)location + size_in_bytes;
for (page = (uint8_t *)location; page < end; page += page_size)
	{
	/*
		The first page is special - we need that page to keep track of the others
		so we pass it to the kernel allocator and bung it on the in-use list
	*/
	if (page_count == 0)
		{
		/*
			First we need to slice the ATOSE "identity" page table out of this
			The page table must be alligned on a 16KB boundary.  As we have 1MB
			pages we can guarantee that the page table will be correctly alliged
			if placed at the start of a page.

			Note that we have not set up the page table yet
		*/
		identity_page_table = (uint32_t *)page;

		/*
			The remainder is available for use as kernel allocatable memory
		*/
		set_allocation_page(page + (pages_in_address_space * sizeof(uint32_t)), page_size - (pages_in_address_space * sizeof(uint32_t)));
		}

	/*
		create a MMU page object
	*/
	current = (ATOSE_mmu_page *)malloc(sizeof(*current));
	current->physical_address = page;
	current->page_size = page_size;

	/*
		bung it on the free list (the first page is in-use storing its own address)
	*/
	if (page_count != 0)
		free_list.push(current);

	/*
		and mark the fact that we have that page
	*/
	page_count++;
	}
}

/*
	ATOSE_MMU::PUSH()
	-----------------
*/
void ATOSE_mmu::push(ATOSE_mmu_page *page)
{
free_list.push(page);
}

/*
	ATOSE_MMU::PULL()
	-----------------
	return NULL if there are no blank pages
*/
ATOSE_mmu_page *ATOSE_mmu::pull(void)
{
ATOSE_mmu_page *page;

/*
	Get the next blank page (if there is one)
*/
if ((page = free_list.pull()) == 0)
	return NULL;		// We're out of memory

/*
	Zero the page (someone is bound to complain about security if we don't do this)
*/
bzero(page->physical_address, page->page_size);

return page;
}

