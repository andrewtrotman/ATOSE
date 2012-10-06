/*
	MMU_PAGE_LIST.H
	---------------
*/
#ifndef MMU_PAGE_LIST_H_
#define MMU_PAGE_LIST_H_

class ATOSE_mmu_page;

/*
	class ATOSE_MMU_PAGE_LIST
	-------------------------
*/
class ATOSE_mmu_page_list
{
private:
	static const uint32_t kernel_process_id = 0;

private:
	ATOSE_mmu_page *top_of_stack;

public:
	ATOSE_mmu_page_list() { top_of_stack = 0; }

	void push(ATOSE_mmu_page *page, uint32_t owning_process = kernel_process_id);
	ATOSE_mmu_page *pull(void);
};

#endif /* MMU_PAGE_LIST_H_ */
