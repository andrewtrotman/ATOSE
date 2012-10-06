/*
	MMU_V5_CONSTANTS.H
	------------------
	Constants (etc) for the ARM architecture v5 MMU used by the ARM926EJ-S
	For more information on the MMU see Chapter 5 of "ARM926EJ-S Technical Reference Manual (Revision: r0p5)"

	For ATOSE we're only going to worry about Sections (1MB pages)
*/
#ifndef MMU_V5_CONSTANTS_H_
#define MMU_V5_CONSTANTS_H_

/*
	The type of the page is stored in the bottom two bits
*/
#define ARM_MMU_V5_PAGE_TYPE_FAULT   0x00
#define ARM_MMU_V5_PAGE_TYPE_COURSE  0x01
#define ARM_MMU_V5_PAGE_TYPE_SECTION 0x02
#define ARM_MMU_V5_PAGE_TYPE_FILE    0x03

/*
	C and B bits.  They are described on page 4-6 of "ARM926EJ-S Technical Reference Manual (Revision: r0p5)"
	C B  Meaning
	0 0  Noncacheable, nonbufferable (DCache disabled. Read from external memory. Write as a nonbuffered store(s) to external memory. DCache is not updated)
	0 1  Noncacheable, bufferable    (DCache disabled. Read from external memory. Write as a buffered store(s) to external memory. DCache is not updated)
	1 0  Write-through               (DCache enabled,  on a Write hit:  Write to the DCache, and buffered store to external memory)
	1 1  Write-back                  (DCache enabled,  on a Write hit:  Write to the DCache only)
*/
#define ARM_MMU_V5_PAGE_NONCACHED_NONBUFFERED (0x00 << 2)
#define ARM_MMU_V5_PAGE_NONCACHED_BUFFERED    (0x01 << 2)
#define ARM_MMU_V5_PAGE_CACHED_WRITE_THROUGH  (0x02 << 2)
#define ARM_MMU_V5_PAGE_CACHED_WRITE_BACK     (0x03 << 2)

/*
	The domain bits (this is simply a 4 bit domain number)
	On the ARM v5 MMU each page descriptor includes an ID (pointer) to a domain.  That domain
	is an authority domain that says who has what permissions on the page.  The domains are
	kept in a seperate register (CP15, register 3)
*/
#define ARM_MMU_V5_PAGE_DOMAIN_00 (0x00 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_01 (0x01 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_02 (0x02 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_03 (0x03 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_04 (0x04 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_05 (0x05 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_06 (0x06 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_07 (0x07 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_08 (0x08 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_09 (0x09 << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0A (0x0A << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0B (0x0B << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0C (0x0C << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0D (0x0D << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0E (0x0E << 5)
#define ARM_MMU_V5_PAGE_DOMAIN_0F (0x0F << 5)

/*
	The allowable domains stored in the domain register (CP15, register 3) are below.  These
	bits are repeated 16 times in the register making a 32-bit domain register indexed by the
	domain number (above)
*/
#define ARM_MMU_V5_DOMAIN_NO_ACCESS 0x00		// any access results in a domain fault
#define ARM_MMU_V5_DOMAIN_CLIENT    0x01		// check for access permissions in the page descriptor (page table)
#define ARM_MMU_V5_DOMAIN_RESERVED  0x02		// Reserved
#define ARM_MMU_V5_DOMAIN_MANAGER   0x03		// no check - a fault cannot occur

#define ARM_MMU_V5_DOMAIN_BITS_PER_DOMAIN 2

/*
	If the domain tells us to do a check (i.e. ARM_MMU_V5_DOMAIN_CLIENT) then the AP bits
	get checked,  If they are 0x00 then its is assumed we're in a "special page" and
	then the ROM and SYSTEM bits (R and S) of CP15 register 1 are checked to see if
	we're in a special mode.  This looks pointless for us.  Instead we'll stick with
	the AP bits being non-zero (which does not check the R and S bits)

	AP
	00  Check the R and S bits (we'll ignore)
	01  Privileged: read write, User: forbidden
	02  Privileged: read write, User: read only
	03  Privileged: read write, User: read write

	We are concerned with sectons because they are 1MB in size and any smaller is 
	unlikely to be useful to us.
*/
#define ARM_MMU_V5_PAGE_SECTION_USER_SPECIAL   (0x00 << 10)
#define ARM_MMU_V5_PAGE_SECTION_USER_FORBIDDEN (0x01 << 10)
#define ARM_MMU_V5_PAGE_SECTION_USER_READONLY  (0x02 << 10)
#define ARM_MMU_V5_PAGE_SECTION_USER_READWRITE (0x03 << 10)

/*
	a "blank" section page has certain bits that must be zero, should be zero,
	must be 1 and shoudl be 1.  Heres the magic number containing those bits
*/
#define ARM_MMU_V5_PAGE (1 << 4)

/*
	Bits in CP15 register 1
*/
#define ARM_MMU_V5_CP15_R1_M  0x0001		// 1 = enable MMU
#define ARM_MMU_V5_CP15_R1_A  0x0002		// 1 = enable data alignment faults
#define ARM_MMU_V5_CP15_R1_C  0x0004		// 1 = enable data cache
#define ARM_MMU_V5_CP15_R1_B  0x0008		// 1 = big-endian 0 = little-endian
#define ARM_MMU_V5_CP15_R1_S  0x0100		// 1 = in system code (I think this has been depricated)
#define ARM_MMU_V5_CP15_R1_R  0x0200		// 1 = in ROM code (I think this has been depricagted)
#define ARM_MMU_V5_CP15_R1_I  0x1000		// 1 = enable instruction cache
#define ARM_MMU_V5_CP15_R1_V  0x2000		// 1 = interrupt vectors at 0x0000FFFF.  0 = interrupt vectors at 0x00000000
#define ARM_MMU_V5_CP15_R1_RR 0x4000		// 1 = Round robin cache replacement. 0 = random replacement
#define ARM_MMU_V5_CP15_R1_L4 0x8000		// 1 = ignore T-bit. 0 = loads to PC set the T-bit

#endif /* MMU_V5_CONSTANTS_H_ */
