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
	By the time we get to ARM v7 there's a no-execute but that's
	supposed to be applied to pages outside the code segment to
	prevent malicious attack.
*/
#define ARM_MMU_V7_PAGE_SECTION_USER_NO_EXECUTE (1 << 4)

/*
	Bits in CP15 register 1
*/
#define ARM_MMU_V5_CP15_R1_M    (1 << 0)		// 1 = enable MMU
#define ARM_MMU_V5_CP15_R1_A    (1 << 1)		// 1 = enable data alignment faults
#define ARM_MMU_V5_CP15_R1_C    (1 << 2)		// 1 = enable data cache
#define ARM_MMU_V5_CP15_R1_B    (1 << 7)		// 1 = big-endian 0 = little-endian
#define ARM_MMU_V5_CP15_R1_S    (1 << 8)		// 1 = in system code (I think this has been depricated)
#define ARM_MMU_V5_CP15_R1_R    (1 << 9)		// 1 = in ROM code (I think this has been depricagted)
#define ARM_MMU_V7_CP15_R1_SW   (1 << 10)		// SWP and SWPB enable
#define ARM_MMU_V7_CP15_R1_Z    (1 << 11)		// Branch prediction enable
#define ARM_MMU_V5_CP15_R1_I    (1 << 12)		// 1 = enable instruction cache
#define ARM_MMU_V5_CP15_R1_V    (1 << 13)		// 1 = interrupt vectors at 0x0000FFFF.  0 = interrupt vectors at 0x00000000
#define ARM_MMU_V5_CP15_R1_RR   (1 << 14)		// 1 = Round robin cache replacement. 0 = random replacement
#define ARM_MMU_V5_CP15_R1_L4   (1 << 15)		// 1 = ignore T-bit. 0 = loads to PC set the T-bit
#define ARM_MMU_V7_CP15_R1_HA   (1 << 17)		// Hardware Access flag enable
#define ARM_MMU_V7_CP15_R1_WXN  (1 << 19)		// Write permission implies XN
#define ARM_MMU_V7_CP15_R1_UWXN (1 << 20)		// Unprivileged write permission implies PL1 XN
#define ARM_MMU_V7_CP15_R1_F1   (1 << 21)		// Fast interrupts configuration enable
#define ARM_MMU_V7_CP15_R1_U    (1 << 22)		// In ARMv7 this bit is RAO/SBOP, indicating use of the alignment model
#define ARM_MMU_V7_CP15_R1_VE   (1 << 24)		// Interrupt Vectors Enable
#define ARM_MMU_V7_CP15_R1_EE   (1 << 25)		// Exception Endianness
#define ARM_MMU_V7_CP15_R1_NMFI (1 << 27)		// Non-maskable FIQ (NMFI) support
#define ARM_MMU_V7_CP15_R1_TRE  (1 << 28)		// TEX remap enable
#define ARM_MMU_V7_CP15_R1_AFT  (1 << 29)		// Access flag enable
#define ARM_MMU_V7_CP15_R1_TE   (1 << 30)		// Thumb Exception enable



#endif /* MMU_V5_CONSTANTS_H_ */
