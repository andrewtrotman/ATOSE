/*
	MMU.C
	-----
*/
#include "atose.h"					// DELETE ME
#include "mmu.h"
#include "mmu_v5_constants.h"
#include "mmu_page_list.h"
#include "mmu_page.h"
#include "address_space.h"
#include "ascii_str.h"

/*
	ATOSE_MMU::INIT()
	-----------------
*/
void ATOSE_mmu::init(void)
{
uint32_t current;

/*
	Turn off the ROM and SYSTEM bits.  These bits appear to be
	depricated in later versions of the ARM MMU
*/
asm volatile
	(
	"mrc	p15, 0, r0, c1, c0;"			// read CP15 Register 1
	"and	r0, r0, %[on_bits];"			// turn off the 'S' and 'R' bits
	"mcr	p15, 0, r0, c1, c0, 0;"			// write the value back to CP5 Register 1
	:
	:[on_bits]"r"(~(ARM_MMU_V5_CP15_R1_S | ARM_MMU_V5_CP15_R1_R))
	:"r0"
	);

/*
	Program the domain register

	We really only need one domain - a domain that says we should check
	all accesses against its permission bits.  It is not clear (to me)
	whether or not there is a cycle cost to a permission check but as the
	page table entry is likely to be cached it is and the check is
	simple, I guess not.  Please, someone, if you find that there is then
	let me know and we'll fix this code.

	If we use domain 0x00 then in the case of an uninitialised 0 it'll
	have meaning.  If we use 0x0F then the same is true (and of 0x01).
	So, we'll use a domain position that is unlikely to occur by accident
	and reserve 0x00, 0x01 and 0xFF as faults.  Arbritrerily, 0x02 seems
	reasonable (ATOSE_mmu::domain), and as each domain takes 2 bits we
	shift it left first.
*/
asm volatile
	(
	"mov	r0, %[domain_string];"				// value to shove in the domain register
	"mcr	p15, 0, r0, c3, c0, 0;"			// write this to CP15 Control Register 3 (the Domain Access Control Register)
	:
	: [domain_string]"r"(ARM_MMU_V5_DOMAIN_CLIENT << (domain * ARM_MMU_V5_DOMAIN_BITS_PER_DOMAIN))
	: "r0"
	);

/*
	Set up a few constants we're going to need later
*/
bad_page = 0;		// cause a fault

/*
	Operating System pages have the low bits set to no cache, no buffer
	and user can read only.  We cannot cache or buffer because the DMA
	controller writes into these pages
		(ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
os_page = (ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*

	Memory Mapped Register pages have the low bits set to no cache, no
	buffer, user forbidden:
		(ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_FORBIDDEN | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/

// if we use a model of turning off the MMU in kernel space then we never need to mark these pages

/*
	User DATA pages are set to cache, buffer, user read write:
		 (ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
user_data_page = (ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	User CODE (probram) pages are set to cache, buffer, and user can read
	only:
		(ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
user_code_page = (ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	A page table can now be set up with:
	each page is 1MB in size (ARM V5 Sections)
	non-existant pages are set to fault on access
		0x00000000
*/
for (current = 0; current < pages_in_address_space; current++)
	identity_page_table[current] = (current << 20) | (ARM_MMU_V5_PAGE | ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	Set the page table to the identity page table
*/
assume(identity_page_table);

/*
	Enable the data cache, the instructon cache, and the MMU.

	NOTE: the data cache and instruction cache are addressed on virtual address
	and therefore need to be flushed on each context switch

	As the cache starts inactive it should not be necessart to flush it
	but we will anyway.
*/
flush_caches();
asm volatile
	(
	"mrc	p15, 0, r0, c1, c0, 0;"			// read c1
	"orr	r0, r0, %[cache_enable];"		// enable data cache and instruction cache
	"mcr	p15, 0, r0, c1, c0, 0;"			// write c1
	"nop;"									// flush the pipeline (probably not necessary)
	"nop;"
	"nop;"
	"nop;"									// flush the pipeline (probably not necessary)
	"nop;"
	"nop;"
	"nop;"
	"nop;"
	:
	: [cache_enable]"r"(ARM_MMU_V5_CP15_R1_C | ARM_MMU_V5_CP15_R1_I | ARM_MMU_V5_CP15_R1_M)
	: "r0"
	);
}

/*
	ATOSE_MMU::FLUSH_CACHES()
	-------------------------
*/
void ATOSE_mmu::flush_caches(void)
{
/*
	The cache on the ARM926 is addressed by virtual address not physical address. This is
	apparently faster than using physical addresses because no address translation is required
	to get entries out of the cache, but it means that every time a context switch occurs
	its necessary to invalidate both the instruction and data cache.
*/
asm volatile
	(
	"mov	r0, #0;"				// the register should be zero
	"mcr p15, 0, r0, c7, c10, 4;"	// drain the write buffer
	"mcr p15, 0, r0, c7, c7, 0;"	// invalidate the data cache and the instruction cache
	:
	:
	: "r0"
	);
}

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
if ((page = free_list.pull()) == NULL)
	return NULL;		// We're out of memory

/*
	Zero the page (someone is bound to complain about security if we don't do this)
*/
bzero(page->physical_address, page->page_size);

return page;
}

/*
	ATOSE_MMU::ASSUME()
	-------------------
*/
void ATOSE_mmu::assume(uint32_t *page_table)
{
asm volatile
	(
	"mov	r0, #0;"						// zero r0
	"mov	r1, %[table];"					// place the address of page table into r1
	"mcr 	p15, 0, r0, c8, c7, 0;"			// clear the CPU TLB
	"mcr	p15, 0, r1, c2, c0;"			// set the Translation Table Base Register
	"mcr 	p15, 0, r0, c8, c7, 0;"			// clear the CPU TLB
	:
	: [table]"r"(page_table)
	: "r0", "r1"
	);
}

/*
	ATOSE_MMU::ASSUME()
	-------------------
*/
void ATOSE_mmu::assume(ATOSE_address_space *address_space)
{
flush_caches();
assume(address_space->get_page_table());
flush_caches();
}

/*
	ATOSE_MMU::ASSUME_IDENTITY()
	----------------------------
*/
void ATOSE_mmu::assume_identity(void)
{
flush_caches();
assume(identity_page_table);
flush_caches();
}
