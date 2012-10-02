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
	//	free_list.push(page);
	}
}

