/*
	MMU_PAGE_LIST.C
	---------------
*/
#include "mmu_page.h"
#include "mmu_page_list.h"

/*
	ATOSE_MMU_PAGE_LIST::PUSH()
	---------------------------
*/
void ATOSE_mmu_page_list::push(ATOSE_mmu_page *page)
{
/*
	Keep a simple stack of pages
*/
page->next = top_of_stack;
top_of_stack = page;
}

/*
	ATOSE_MMU_PAGE_LIST::PULL()
	---------------------------
*/
ATOSE_mmu_page *ATOSE_mmu_page_list::pull(void)
{
ATOSE_mmu_page *answer;

/*
	We have a simple stack of pages
*/
if ((answer = top_of_stack) != 0)
	top_of_stack = top_of_stack->next;

return answer;
}
