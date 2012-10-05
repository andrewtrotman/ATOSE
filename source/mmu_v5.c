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
	Turn off the ROM and SYSTEM bits
*/
asm volatile
	(
	"mrc	p15, 0, r0, c1, c0;"			// read CP15 Register 1
	"orr	r0, r0, #0x1000;"				// turn off the 'S' and 'R' bits
	"mcr	p15, 0, r0, c1, c0, 0;"			// write the value back to CP5 Register 1
	:
	:
	:"r0"
	);

/*
	Program the domain register
*/
/*
	We really only need one domain - a domain that says we should check
	all accesses against its permission bits.  It is not clear (to me)
	whether or not there is a cycle cost to a permission check but as the
	page table entry is likely to be cached it is and the check is
	simple, I guess not.  Please, someone, if you find that there is then
	let me know and we'll fix this code.
*/
asm volatile
	(
	"mov	r0, %[domains];"				// value to shove in the domain register
	"mcr	p15, 0, r0, c3, c0, 0;"			// write this to CP15 Control Register 3 (the Domain Access Control Register)
	:
	: [domains]"I"(ARM_MMU_V5_DOMAIN_CLIENT)
	:"r0"
	);
}


