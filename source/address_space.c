/*
	ADDRESS_SPACE.C
	---------------

	In ATOSE we assume a model of a thread being the abstraction of the
	CPU and the address space being the abstraction of the memory.  A
	process, therefore consists of two seperat abstractions - one for the
	memory, one for the	CPU and one for memory.  This code represents
	just the address space.
*/
#include "mmu.h"
#include "mmu_page.h"
#include "address_space.h"

/*
	ATOSE_ADDRESS_SPACE::CREATE()
	-----------------------------
*/
ATOSE_address_space *ATOSE_address_space::create(void)
{
ATOSE_mmu_page *page, *stack_page;
uint32_t current;

/*
	Get a page from the MMU and use that page as the page table. In that
	page table we put the OS at the bottom (because that's were the
	interrupt vector table must be stored) the page table itself is not
	in the address space.  Initialise  all other pages to cause faults
	except the stack at top of memory, which requires the allocation of a
	second page.
*/
if ((page = mmu->pull()) == 0)
	return 0;			// fail as we're of out of physical pages to allocate

page_table = ((uint32_t *)page->physical_address);
page_list.push(page);		// mark the page as part of the process's list of pages

/*
	Place ATOSE at bottom of memory
*/
page_table[0] = 0 | mmu->os_page;

/*
	Mark all other pages to cause faults (except for the last page)
*/
for (current = 1; current < mmu->pages_in_address_space - 1; current++)
	page_table[current] =  mmu->bad_page;

/*
	Allocate the stack at top of memory
*/
if ((stack_page = mmu->pull()) == 0)
	return 0;		// clean up will happen when the address_space object is deleted

add_page((void *)(mmu->highest_address - mmu->page_size + 1), stack_page, mmu->user_data_page);

return this;
}

/*
	ATOSE_ADDRESS_SPACE::CREATE_IDENTITY()
	--------------------------------------
	Create an address space that is the identity address space
*/
ATOSE_address_space *ATOSE_address_space::create_identity(void)
{
/*
	first create the address space
*/
create();

/*
	then set the page table to the identity page table
*/
page_table = mmu->get_identity_page_table();

return this; 
}

/*
	ATOSE_ADDRESS_SPACE::DESTROY()
	------------------------------
	returns 0 on success and anything else on failure
*/
uint32_t ATOSE_address_space::destroy(void)
{
ATOSE_mmu_page *page;

/*
	Hand all the memory back
*/
for (page = page_list.pull(); page != NULL; page = page_list.pull())
	mmu->push(page);

return 0;
}

/*
	ATOSE_ADDRESS_SPACE::ADD_PAGE()
	-------------------------------
	virtual_address must be page-aligned
	return 0 on success
*/
uint32_t ATOSE_address_space::add_page(void *virtual_address, ATOSE_mmu_page *page, uint32_t type)
{
size_t page_table_entry;

/*
	Work out which entry in the page table we are
*/
page_table_entry = (((size_t)virtual_address) / mmu->page_size);

/*
	Mark the page in the page table and set its type
*/
page_table[page_table_entry] = (size_t)(page->physical_address) | type;

/*
	Add the page to our in-use list
*/
page_list.push(page);

/*
	Success
*/
return 0;
}

/*
	ATOSE_ADDRESS_SPACE::ADD()
	--------------------------
*/
uint8_t *ATOSE_address_space::add(void *address, size_t size, uint32_t permissions)
{
ATOSE_mmu_page *page;
size_t base_page, last_page, which;
uint64_t end;

/*
	Make sure we fit within the address space
*/
if ((end = ((size_t)address + size)) > mmu->highest_address)
	return NULL;					// we over-flow the address space

/*
	Get the page number of the first page in the list and the number of pages needed
*/
base_page = (((size_t)address) / mmu->page_size);
last_page = (end / mmu->page_size);

/*
	Now get pages and add them to the page table
*/
for (which = base_page; which <= last_page; which++)
	{
	/*
		Verify that the page isn't already in the address space and ignore that page if so.
	*/
	if (page_table[which] == 0)
		{
		if ((page = mmu->pull()) == 0)
			return NULL;		// We fail because there are no more pages to give
		else
			add_page((void *)(which * mmu->page_size), page, mmu->user_data_page);		// FIX the permissions here are wrong.
		}
	}

/*
	return a pointer to the beginning of the address space allocated
*/
return (uint8_t *)address;
}
