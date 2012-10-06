/*
	MMU.C
	-----
*/
#include "mmu.h"
#include "mmu_page_list.h"
#include "mmu_page.h"

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
		set_allocation_page(page, page_size);

	/*
		create a MMU page object
	*/
	current = (ATOSE_mmu_page *)malloc(sizeof(*current));
	current->physical_address = page;

	/*
		bung it on the right list (the first page is in-use storing its own address)
	*/
	if (page_count == 0)
		in_use_list.push(current);
	else
		free_list.push(current);

	/*
		and mark the fact that we have that page
	*/
	page_count++;
	}
}

/*
	ATOSE_MMU::CREATE_KERNEL_PAGE_TABLE()
	-------------------------------------
*/
void ATOSE_mmu::create_kernel_page_table(uint8_t *page)
{
uint32_t *page_table;
uint32_t current;

/*
	Create the identity page table
*/
for (current = 0; current < pages_in_address_space; current++)
	page_table[current] = 0;

/*
	Mark page 0 as the location of the kernel
*/
page_table[0] = 0;
}
