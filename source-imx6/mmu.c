/*
	MMU.C
	-----
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "mmu.h"
#include "mmu_v5_constants.h"
#include "mmu_page_list.h"
#include "mmu_page.h"
#include "address_space.h"
#include "ascii_str.h"

/*
	ATOSE_MMU::INITIALISE()
	-----------------------
*/
void ATOSE_mmu::initialise(void)
{
uint32_t current;

/*
	Don't even ask what this does...  Just go read pages 2-47 to 2-48 of "ARM Cortex-A9 processors r2 releases"
*/
asm volatile
	(
	"mrc p15, 0, r0, c15, c0, 1;"
	"orr r0, r0, #0x40;"
	"mcr p15, 0, r0, c15, c0, 1;"
	:
	:
	: "r0"
	);

/*
	Turn off the ROM and SYSTEM bits.  These bits appear to be
	depricated in later versions of the ARM MMU anyway
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
	"mov	r0, %[domain_string];"			// value to shove in the domain register
	"mcr	p15, 0, r0, c3, c0, 0;"			// write this to CP15 Control Register 3 (the Domain Access Control Register)
	:
	: [domain_string]"r"(ARM_MMU_V5_DOMAIN_CLIENT << (domain * ARM_MMU_V5_DOMAIN_BITS_PER_DOMAIN))
	: "r0"
	);

/*
	Set up a few constants we're going to need later
*/
bad_page = ARM_MMU_V5_PAGE_TYPE_FAULT;		// cause a fault

/*
	Operating System pages have the low bits set to no cache, no buffer
	and user can read only.  We cannot cache or buffer because the DMA
	controller writes into these pages
		(ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
//os_page = (ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION);
os_page = ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION;

/*

	Memory Mapped Register pages have the low bits set to no cache, no buffer, user forbidden:
		(ARM_MMU_V5_PAGE_SECTION_USER_FORBIDDEN | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
peripheral_page = (ARM_MMU_V5_PAGE_SECTION_USER_FORBIDDEN | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	User DATA pages are set to cache, buffer, user read write, no-execute:
		 (ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V7_PAGE_SECTION_USER_NO_EXECUTE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
user_data_page = (ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V7_PAGE_SECTION_USER_NO_EXECUTE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	User CODE (probram) pages are set to cache, buffer, and user can read only:
		(ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_CACHED_WRITE_BACK | ARM_MMU_V5_PAGE_TYPE_SECTION)
*/
user_code_page = (ARM_MMU_V5_PAGE_SECTION_USER_READONLY | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	A page table can now be set up with: each page is 1MB in size (ARM V5 Sections), full permission to do anything
*/
for (current = 0; current < pages_in_address_space; current++)
	identity_page_table[current] = (current << 20) | (ARM_MMU_V5_PAGE_SECTION_USER_READWRITE | ARM_MMU_V5_PAGE_DOMAIN_02 | ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED | ARM_MMU_V5_PAGE_TYPE_SECTION);

/*
	page 6-2 of "Cortex-A9 Technical Reference Manual Revision: r2p0"
	"You must invalidate the instruction cache, the data cache, and BTAC before using them.
	You are not required to invalidate the main TLB, even though it is recommended for
	safety reasons. This ensures compatibility with future revisions of the processor"

	Where BTAC is "Branch Target Address Cache"
*/
invalidate_data_cache();		// invalidate because at boot it could contain noise
flush_caches();				// now flush

/*
	Set the page table to the identity page table
*/
assume(identity_page_table);

/*
	Enable the data cache, the instructon cache, branch prediction, and the MMU.

	NOTE: the data cache and instruction cache are addressed on virtual address
	and therefore need to be flushed on each context switch
*/
asm volatile
	(
	"mrc	p15, 0, r0, c1, c0, 0;"			// read c1
	"orr	r0, r0, %[cache_enable];"		// enable data cache and instruction cache
	"mcr	p15, 0, r0, c1, c0, 0;"			// write c1
	"dsb;"									// full barrier
	:
	: [cache_enable]"r"(ARM_MMU_V5_CP15_R1_C | ARM_MMU_V5_CP15_R1_I | ARM_MMU_V7_CP15_R1_Z | ARM_MMU_V5_CP15_R1_M)
	: "r0"
	);

initialised = true;
}

/*
	ATOSE_MMU::INVALIDATE_DATA_CACHE()
	----------------------------------
*/
void ATOSE_mmu::invalidate_data_cache(void)
{
/*
	At startup the data cache is full of garbage so we need to invalidate it
	without flushing it to main memory because that might trach main memory.

	The code for the i.MX6Q (ARM Cortex A9) is somewhat more involved.  For more detail on exactly how to
	do this see pages 15-3 to 15-4 of "Cortex-A Series Programmer's Guide Version: 3.0" from where this
	code was taken.
*/
asm volatile
	(
	/*
		Data cache invalidate all
	*/
	"mrc p15, 1, r0, c0, c0, 0;"		// Read Cache Size ID
	"movw r3, #0x1ff;"
	"and r0, r3, r0, lsr #13;"			// r0 = no. of sets - 1
	"mov r1, #0;"						// r1 = way counter way_loop
"idc_way_loop:;"
	"mov r3, #0;"						// r3 = set counter set_loop
"idc_set_loop:;"
	"mov r2, r1, lsl #30;"
	"orr r2, r3, lsl #5;"				// r2 = set/way cache operation format
	"mcr p15, 0, r2, c7, c6, 2;"		// DCISW - Invalidate line described by r2
	"add r3, r3, #1;"					// Increment set counter
	"cmp r0, r3;"						// Last set reached yet?
	"bne idc_set_loop;"						// if not, iterate set_loop
	"add r1, r1, #1;"					// else, next
	"cmp r1, #4;"						// Last way reached yet?
	"bne idc_way_loop;"						// if not, iterate way_loop
	:
	:
	:"r0", "r1", "r2", "r3");
}

/*
	ATOSE_MMU::FLUSH_CACHES()
	-------------------------
*/
void ATOSE_mmu::flush_caches(void)
{
/*
	The cache on the ARM926 is addressed by virtual address not physical
	address. This is apparently faster than using physical addresses
	because no address translation is required to get entries out of the
	cache, but it means that every time a context switch occurs its
	necessary to invalidate both the instruction and data cache.

	As writes cannot happen to the instruction cache, ICache, we just
	invalidate it.  The data cache, DCache, however can have dirty data
	in it.  We need to flush this to memory.  To do that we "test and
	clean" the cache which checks for dirty data and flushes it to
	memory.  We then need to invalidate the cache.  As there's a test,
	clean, and invalidate instruction we just use that.  Note (from page
	2-23 of the ARM926EJ-S Technical Reference Manual) that we need to
	check each line in a loop.

	The cache is, of course, not actually flushed to memory, but is
	rather flushed to the write buffer.  To truely flush the cache its
	necessary to also flush the write buffer.
*/

/*
	The code for the i.MX6Q (ARM Cortex A9) is somewhat more involved.  For more detail on exactly how to
	do this see pages 15-3 to 15-4 of "Cortex-A Series Programmer's Guide Version: 3.0" from where this
	code was taken.
*/
asm volatile
	(
	/*
		Barrier (complete all instructions up to this point before going on)
	*/
	"dsb;"

	/*
		Clear register 0
	*/
	"mov r0, #0;"

	/*
		Invalidate the TLB
	*/
	"mcr p15, 0, r0, c8, c7, 0;"		// TLBIALL

	/*
		Branch predictor invalidate all
	*/
	"mcr p15, 0, r0, c7, c5, 6;"		// BPIALL

	/*
		Instruction cache invalidate all
	*/
	"mcr p15, 0, r0, c7, c5, 0;"		// ICIALLU
	"isb;"									// flush the CPU pipeline

	/*
		Data cache invalidate all
	*/
	"mrc p15, 1, r0, c0, c0, 0;"		// Read Cache Size ID
	"movw r3, #0x1ff;"
	"and r0, r3, r0, lsr #13;"			// r0 = no. of sets - 1
	"mov r1, #0;"						// r1 = way counter way_loop
"way_loop:;"
	"mov r3, #0;"						// r3 = set counter set_loop
"set_loop:;"
	"mov r2, r1, lsl #30;"
	"orr r2, r3, lsl #5;"				// r2 = set/way cache operation format
	"mcr p15, 0, r2, c7, c14, 2;"		// DCCISW - Clean and Invalidate line described by r2
	"add r3, r3, #1;"					// Increment set counter
	"cmp r0, r3;"						// Last set reached yet?
	"bne set_loop;"						// if not, iterate set_loop
	"add r1, r1, #1;"					// else, next
	"cmp r1, #4;"						// Last way reached yet?
	"bne way_loop;"						// if not, iterate way_loop


	/*
		Barrier (complete all instructions up to this point before going on)
	*/
	"dsb;"

	:
	:
	:"r0", "r1", "r2", "r3");
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
			First we need to slice the ATOSE "identity" page table out of this.
			The page table must be aligned on a 16KB boundary.  As we have 1MB
			pages we can guarantee that the page table will be correctly aligned
			if placed at the start of a page.

			Note that we have not set up the page table yet
		*/
		identity_page_table = (uint32_t *)page;

		/*
			The remainder is available for use as kernel allocatable memory
		*/
		set_allocation_page(page + (pages_in_address_space * sizeof(uint32_t)), page_size - (pages_in_address_space * sizeof(uint32_t)));
		}
	else if (page_count == 1)
		{
		/*
			Take a note of where the system's "break" should be.
		*/
		the_system_break = page;
		}

	/*
		create an MMU page object
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
//bzero(page->physical_address, page->page_size);

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
	"mcr	p15, 0, %[table], c2, c0, 0;"	// set the Translation Table Base Register
	"dsb;"										// make sure it completes before proceeding
	:
	: [table]"r"(page_table)
	:
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
}

/*
	ATOSE_MMU::ASSUME_IDENTITY()
	----------------------------
*/
void ATOSE_mmu::assume_identity(void)
{
if (initialised)
	{
	flush_caches();
	assume(identity_page_table);
	}
}
