/*
	MMU.C
	-----
*/
#include "mmu.h"
#include "mmu_page_list.h"

/*
	ATOSE_MMU::PUSH()
	-----------------
*/
void ATOSE_mmu::push(void *location, uint64_t size_in_bytes)
{
uint8_t *page, *end;

end = (uint8_t *)location + size_in_bytes;
for (page = (uint8_t *)location; page < end; page += page_size)
	{
	/*
		FIX THIS CODE
	*/
#ifdef NEVER
	/*
		The first page is special - we need that to keep track of the others
	*/
	if (page_count == 0)
		{
		/*
			Map this into the Kernel address space
			mark it as used
			then allocate from it
		*/
		in_use_list->push(page);
		}
	else
		free_list.push(page);
#endif
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

/*
	Mark the first page as where the kernel is
*/
page_table[0] = 0;
}
