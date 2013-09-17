/*
	ADDRESS_SPACE.C
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	In ATOSE we assume a model of a thread being the abstraction of the
	CPU and the address space being the abstraction of the memory.  A
	process, therefore consists of two seperat abstractions - one for the
	memory, one for the	CPU and one for memory.  This code represents
	just the address space.
*/
#include "atose.h"
#include "mmu.h"
#include "mmu_page.h"
#include "address_space.h"

#include "debug_kernel.h"

/*
	ATOSE_ADDRESS_SPACE::CREATE()
	-----------------------------
*/
ATOSE_address_space *ATOSE_address_space::create(void)
{
ATOSE_address_space *answer = NULL;
ATOSE_mmu_page *page, *stack_page;
uint32_t current;
uint32_t current_page_table;

/*
	There are no open pipes in this address space
*/
open_pipes = NULL;

/*
	Move into the land of the identity address space (so we can access memory directly)
*/
current_page_table = mmu->get_current_page_table();
mmu->assume_identity();

/*
	Get a page from the MMU and use that page as the page table. In that
	page table we put the OS at the bottom (because that's were the
	interrupt vector table must be stored) the page table itself is later placed
	in the address space.  Initialise  all other pages to cause faults
	except the stack at top of memory, which requires the allocation of a
	second page.
*/
if ((page = mmu->pull()) != 0)
	{
	page_table = ((uint32_t *)page->physical_address);
	page_list.push(page);		// mark the page as part of the process's list of pages

	/*
		Place ATOSE at bottom of memory on the i.MX6Q
		On the i.MX6Q the bottom 1MB of of the address space is:
			0x00000000 - 0x00017FFF  96 KB Boot ROM (ROMCP)
			0x00018000 - 0x000FFFFF 928 KB Reserved
			0x00100000 - 0x00103FFF  16 KB CAAM (16K secure RAM)
		Then there's the register map going up to 0x10000000 (the 256MB mark) including
			0x00900000 - 0x0093FFFF 256KN OCRAM
			0x00940000 - 0x009FFFFF 0.75 MB OCRAM aliased
		Its this on-chip RAM where we'll fit the ATIRE (interesting how, including alias, its 1MB (Section size)).  We can mark these pages as user no-access

		On the i.MX6Q the bottom 256 MB of RAM is registers and on-chip RAM so we slice that out of the address space.
	*/

	/*
		The peripheral pages (register map) must be non-bufferable non-cachable
	*/
	for (current = 1; current < 256; current++)
		page_table[current] = (current << 20) | mmu->peripheral_page;			// user access is forbidden

	/*
		The ROM must be executable by the OS because otherwise an IRQ cannot be serviced, but no user read
		The 1MB of OC-RAM including its alias can be cachable, buffereable, etc, but no user read
	*/
	page_table[0] = (0 << 20) | mmu->os_page;
	page_table[9] = (9 << 20) | mmu->os_page;

	/*
		Place the page table into the address space (because we're going to need it for hardware stuff)
	*/
	page_table[current] = (uint32_t)(((uint32_t)page->physical_address & 0xFFF00000) | mmu->os_page);
	soft_page_table = (uint32_t *)(current << 20);
	current++;

	/*
		Set the break and mark all pages above it to cause faults (except for the last page, the stack)
	*/
	the_heap_break = (uint8_t *)(current  << 20);
	while (current < mmu->pages_in_address_space - 1)
		{
		page_table[current] =  mmu->bad_page;		// fault on access
		current++;
		}

	/*
		Allocate the stack at top of memory
	*/
	if ((stack_page = mmu->pull()) == 0)
		answer = NULL;		// clean up will happen when the address_space object is deleted
	else
		{
		the_stack_break = (uint8_t *)(mmu->highest_address - mmu->page_size + 1);
		add_page(the_stack_break, stack_page, mmu->user_data_page);
		the_stack_break -= mmu->page_size;
		answer = this;
		}
	}

/*
	Return to the caller's address space
*/
mmu->set_current_page_table(current_page_table);

return answer;
}

/*
	ATOSE_ADDRESS_SPACE::DESTROY()
	------------------------------
	Returns the remaining thread count, on zero the pages are freed

*/
uint32_t ATOSE_address_space::destroy(void)
{
ATOSE_mmu_page *page;
uint32_t current_page_table;

/*
	Move into the land of the identity address space (so we can access memory directly)
*/
current_page_table = mmu->get_current_page_table();
mmu->assume_identity();

/*
	Decrement the reference count
*/
thread_count--;

/*
	if other objects refer to this
*/
if (thread_count > 0)
	return thread_count;

/*
	Hand all the memory back
*/
for (page = page_list.pull(); page != NULL; page = page_list.pull())
	mmu->push(page);

/*
	Return to the caller's address space
*/
mmu->set_current_page_table(current_page_table);

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
	returns 0 on failure
*/
uint8_t *ATOSE_address_space::add(void *address, size_t size, uint32_t permissions)
{
ATOSE_mmu_page *page;
size_t base_page, last_page, which;
uint64_t end;
uint32_t rights;
uint32_t current_page_table;

/*
	Make sure we fit within the address space
*/
if ((end = ((size_t)address + size)) > mmu->highest_address)
	return NULL;					// we over-flow the address space

/*
	Get into the identity space
*/
current_page_table = mmu->get_current_page_table();
mmu->assume_identity();


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
			{
			address = NULL;		// We fail because there are no more pages to give
			break;
			}

		/*
			Sort out the permissions
		*/
		if (permissions == NONE)
			rights = mmu->bad_page;								// yea, well, no access at all is permitted
		else if ((permissions & EXECUTE) != 0)
			rights = mmu->user_code_page;						// execute implies READ but no WRITE
		else
			rights = mmu->user_data_page;						// else we're a data page and all data pages are READ/WRITE but no EXECUTE

		/*
			add the page to the pool
		*/
		add_page((void *)(which * mmu->page_size), page, rights);
		}
	}

/*
	Return to the caller's address space
*/
mmu->set_current_page_table(current_page_table);

/*
	return a pointer to the beginning of the address space allocated
*/
return (uint8_t *)address;
}

/*
	ATOSE_ADDRESS_SPACE::SET_HEAP_BREAK()
	-------------------------------------
*/
void *ATOSE_address_space::set_heap_break(uint32_t bytes_to_add, uint32_t permissions)
{
add(the_heap_break, bytes_to_add, permissions);
the_heap_break += bytes_to_add;

return the_heap_break;
}
