/*
	MMU_V5.C
	--------
*/
#include "mmu_v5.h"
#include "mmu_v5_constants.h"

/*
	ATOSE_MMU_V5::ATOSE_MMU_V5()
	----------------------------
*/
ATOSE_mmu_v5::ATOSE_mmu_v5()
{
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
	reasonable (ATOSE_mmu_v5::domain), and as each domain takes 2 bits we
	shift it left first.
*/
asm volatile
	(
	"mov	r0, %[domain_string];"				// value to shove in the domain register
	"mcr	p15, 0, r0, c3, c0, 0;"			// write this to CP15 Control Register 3 (the Domain Access Control Register)
	:
	: [domain_string]"r"(ARM_MMU_V5_DOMAIN_CLIENT << (domain << ARM_MMU_V5_DOMAIN_BITS_PER_DOMAIN))
	:"r0"
	);

/*
	Enable the data cache and the instructon cache.

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
	:
	: [cache_enable]"r"(ARM_MMU_V5_CP15_R1_C | ARM_MMU_V5_CP15_R1_I)
	: "r0"
	);

/*
	A page table can now be set up with:
	each page is 1MB in size (ARM V5 Sections)
	non-existant pages are set to fault on access
		0x00000000
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
	NOTE: At this point we've not yet actually turned on the MMU, but we have turned on the caches
*/
}

/*
	ATOSE_MMU_V5::FLUSH_CACHES()
	----------------------------
*/
void ATOSE_mmu_v5::flush_caches(void)
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

