/*
	CPU.C
	-----
*/

#include <stdio.h>

/*
	class ATOSE_CPU
	---------------
*/
class ATOSE_cpu
{
protected:
	static void decode_domain_register(int value);
	static void decode_fault_status_register(int cp15);
	static void text_render_decode_cache_size(int cache_size);
	static void decode_p15_TCM_size(int value);

public:
	ATOSE_cpu() {}
	virtual ~ATOSE_cpu() {}

	static void text_render(void);
} ;

/*
	ATOSE_CPU::TEXT_RENDER_DECODE_CACHE_SIZE()
	------------------------------------------
*/
void ATOSE_cpu::text_render_decode_cache_size(int cache_size)
{
const char *string;

if (cache_size & 0x04)		// check the M bit
	switch ((cache_size >> 6) & 0x0F)		// M == 1
		{
		case 0: string = "768B"; break;
		case 1: string = "1536B"; break;
		case 2: string = "3KB"; break;
		case 3: string = "6KB"; break;
		case 4: string = "12KB"; break;
		case 5: string = "24KB"; break;
		case 6: string = "48KB"; break;
		case 7: string = "96KB"; break;
		case 8: string = "192KB"; break;
		default: string = "Unknown"; break;
		}
else
	switch ((cache_size >> 6) & 0x0F)
		{
		case 0: string = "512B"; break;
		case 1: string = "1KB"; break;
		case 2: string = "2KB"; break;
		case 3: string = "4KB"; break;
		case 4: string = "8KB"; break;
		case 5: string = "16KB"; break;
		case 6: string = "32KB"; break;
		case 7: string = "64KB"; break;
		case 8: string = "128KB"; break;
		default: string = "Unknown"; break;
		}

printf("      Size             : %s\n", string);
printf("      Restrictions (P) : %s\n", cache_size >> 11 ? "Some" : "No");
switch (cache_size &0x03)
	{
	case 0: string = "8 bytes"; break;
	case 1: string = "16 bytes"; break;
	case 2: string = "32 bytes"; break;
	case 3: string = "64 bytes"; break;
	}
printf("      Cache lines      : %s\n", string);


if (cache_size & 0x04)		// check the M bit
	{
	switch ((cache_size >> 3) & 0x07)			// M == 1
		{
		case 0: string = "0" ; break;
		case 1: string = "3" ; break;
		case 2: string = "6" ; break;
		case 3: string = "12" ; break;
		case 4: string = "24" ; break;
		case 5: string = "48" ; break;
		case 6: string = "96" ; break;
		case 7: string = "192" ; break;
		}
	}
else
	{
	switch ((cache_size >> 3) & 0x07)
		{
		case 0: string = "1" ; break;
		case 1: string = "2" ; break;
		case 2: string = "4" ; break;
		case 3: string = "8" ; break;
		case 4: string = "16" ; break;
		case 5: string = "32" ; break;
		case 6: string = "64" ; break;
		case 7: string = "128" ; break;
		}
	}
printf("      Associativity    : %s-way\n", string);
}


/*
	DECODE_DOMAIN_REGISTER()
	------------------------
*/
void ATOSE_cpu::decode_domain_register(int value)
{
switch (value & 0x03)
	{
	case 0: printf("No access\n"); break;
	case 1: printf("Client access\n"); break;
	case 2: printf("Reserved\n"); break;
	case 3: printf("Manager\n"); break;
	}
}

/*
	DECODE_FAULT_STATUS_REGISTER()
	------------------------------
*/
void ATOSE_cpu::decode_fault_status_register(int cp15)
{
const char *string;

switch (cp15 & 0x0F)
	{
	case 0: string = "Undefined"; break;
	case 1: string = "Alignment"; break;
	case 2: string = "Undefined"; break;
	case 3: string = "Alignment"; break;
	case 4: string = "Undefined"; break;
	case 5: string = "Translation"; break;
	case 6: string = "Undefined"; break;
	case 7: string = "Translation"; break;
	case 8: string = "External abort"; break;
	case 9: string = "Domain"; break;
	case 10: string = "External abort"; break;
	case 11: string = "Domain"; break;
	case 12: string = "External abort on translation (first level)"; break;
	case 13: string = "Permission"; break;
	case 14: string = "External abort on translation (second level)"; break;
	case 15: string = "Permission"; break;
	}
printf("  Fault Type              : %s\n", string);
switch (cp15 & 0x0F)
	{
	case 7:
	case 9:
	case 11:
	case 13:
	case 14:
	case 15:
		printf("  in Domain               : %X\n", (cp15 >> 4) & 0x0F);
		break;
	}
}

/*
	ATOSE_CPU::DECODE_P15_TCM_SIZE()
	--------------------------------
*/
void ATOSE_cpu::decode_p15_TCM_size(int value)
{
const char *string;

printf("  Enable bit              : %s\n", (value & 0x01) ? "Enabled" : "Disabled");

switch ((value >> 2) & 0x0F)
	{
	case 0: string = "0KB"; break;
	case 1: string = "Reserved"; break;
	case 2: string = "Reserved"; break;
	case 3: string = "4KB"; break;
	case 4: string = "8KB"; break;
	case 5: string = "16KB"; break;
	case 6: string = "32KB"; break;
	case 7: string = "64KB"; break;
	case 8: string = "128KB"; break;
	case 9: string = "256KB"; break;
	case 10: string = "512KB"; break;
	case 11: string = "1MB"; break;
	case 12: string = "Reserved"; break;
	case 13: string = "Reserved"; break;
	case 14: string = "Reserved"; break;
	case 15: string = "Reserved"; break;
	}

printf("  Size                    : %s\n", string);
printf("  Address                 : %04X\n", value >> 12);
}

/*
	ATOSE_CPU::TEXT_RENDER()
	------------------------
*/
void ATOSE_cpu::text_render(void)
{
int cpsr;		// the ARM cpsr program status register
int j, t;		// the values of the J (jazelle) and T (thumb) bits of the cpsr
int cp15;		// result of a call to the CP15 co-processor to get some value
int architecture;		// decoded from the cp15 bits, used to determin the ARM version
int cache_size;		// used to decode Dsize and Isize
const char *string;			// the name of the decoded version of some parameter
int domain;				// current domain number;

/*
	Get the value of the status register and decode it
	see section A.25 of the "ARM Architecture Reference Manual"
*/
asm volatile ("mrs %0, cpsr" : "=r"(cpsr));
printf("CPSR: 0x%08X\n", cpsr);
printf("  Negative   (N): %2X\n", (cpsr >> 31) & 0x01);
printf("  Zero       (Z): %2X\n", (cpsr >> 30) & 0x01);
printf("  Carry      (C): %2X\n", (cpsr >> 29) & 0x01);
printf("  Overflow   (V): %2X\n", (cpsr >> 28) & 0x01);
printf("  Saturation (Q): %2X\n", (cpsr >> 27) & 0x01);
printf("  SIMD >=   (GE): %2X\n", (cpsr >> 16) & 0x0F);

printf("  Jazelle    (J): %2X\n", j = (cpsr >> 24) & 0x01);
printf("  Thumb      (T): %2X\n", t = (cpsr >> 5) & 0x01);
switch ((j << 1) | t)
	{
	case 0x00: printf("    ARM instruction set\n");	break;
	case 0x01: printf("    Thumb instruction set\n");	break;
	case 0x02: printf("    Jazelle (Java) instruction set\n");	break;
	case 0x03: printf("    Unknown instruction set (invalid)\n");	break;
	}

printf("  Endianness (E): %2X (%s)\n", (cpsr >> 9) & 0x01, ((cpsr >> 9) & 0x01) ? "Big Endian" : "Little Endian");
printf("  Data abort (A): %2X (%s)\n", (cpsr >> 8) & 0x01, ((cpsr >> 8) & 0x01) ? "Disabled" : "Enabled");
printf("  IRQ        (I): %2X (%s)\n", (cpsr >> 7) & 0x01, ((cpsr >> 7) & 0x01) ? "Disabled" : "Enabled");
printf("  FIRQ       (F): %2X (%s)\n", (cpsr >> 6) & 0x01, ((cpsr >> 6) & 0x01) ? "Disabled" : "Enabled");
printf("  Mode       (M): 0x%02X\n", cpsr & 0x1F);

switch (cpsr & 0x1F)
	{
	case 0x10: printf("    USER mode\n");	break;
	case 0x11: printf("    FIRQ mode\n");	break;
	case 0x12: printf("    IRQ mode\n");	break;
	case 0x13: printf("    SUPERVISOR mode\n");	break;
	case 0x17: printf("    ABORT mode\n");	break;
	case 0x1C: printf("    UNDEFINED mode\n");	break;
	case 0x1F: printf("    SYSTEM mode\n");	break;
	default:   printf("    mode is unknown and undefined\n");	break;
	}

/*
	Now deal with the CP15 co-processor (mostly memory management stuff)
	register 0 : ID codes
*/
asm("mrc p15, 0, %0, c0, c0, 0" : "=r"(cp15));

printf("\n");
printf("CP15-C0 ID code : 0x%08X\n", cp15);
printf("  Implementer   : 0x%X ('%c')\n",  cp15 >> 24, cp15 >> 24);
if ((cp15 & (1 << 19)) == 0)
	{
	architecture = (cp15 >> 12) & 0x0F;
	if (architecture == 0x00)
		printf("  pre-ARMv4 CPU (obsolete)\n");
	else 
		printf("  ARM V%X\n", architecture);
	}

if (architecture != 0x00 && architecture != 0x07)
	{
	printf("    Variant       : 0x%02X\n", (cp15 >> 20) & 0x0F);
	printf("    Architecture  : 0x%02X\n", (cp15 >> 16) & 0x0F);
	printf("    Part Number   : 0x%03X\n", (cp15 >>  4) & 0xFFF);
	printf("    Revision      : 0x%02X\n", cp15 & 0x0F);
	}


/*
	CP15 register 0, cache type register
*/
asm("mrc p15, 0, %0, c0, c0, 1" : "=r"(cp15));
printf("\n");
printf("CP15-C0 Cache type register: 0x%08X\n", cp15);
printf("    Ctype              : %X (should be 0x0E)\n", (cp15 >> 25) & 0x0F);
printf("    Seperate Caches (S): %X (caches are %s)\n", (cp15 >> 24) % 0x01, (cp15 >> 24) & 0x01 ? "Seperate (Harvard)" : "Unified");
/*
	Data cache
*/
cache_size = (cp15 >> 12) & 0xFFF;
printf("    Dsize              : 0x%03X\n", cache_size);
text_render_decode_cache_size(cache_size);

/*
	Instruction Cache
*/
cache_size = cp15 & 0xFFF;
printf("    Isize              : 0x%03X\n", cache_size);
text_render_decode_cache_size(cache_size);

/*
	CP15 register 0, Tighly coupled memory (TCM) type register
*/
asm("mrc p15, 0, %0, c0, c0, 2" : "=r"(cp15));
printf("\n");
printf("CP15-C0 Tightly Coupled Memory (TCM) register: 0x%08X\n", cp15);
printf("  Banks of Instruction TCM:%d\n", cp15 & 0x7);
printf("  Banks of Data TCM       :%d\n", (cp15 >> 16) & 0x7);

/*
	CP15 register 1, Control Register
*/
asm("mrc p15, 0, %0, c1, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C1 Control Register  : 0x%08X\n", cp15);
printf("  MMU                 (M) : %s\n", (cp15 & 1) ? "Enabled" : "Disabled");
printf("  Alignment faults    (A) : %s\n", (cp15 & 2) ? "Enabled" : "Disabled");
printf("  DCache              (C) : %s\n", (cp15 & 4) ? "Enabled" : "Disabled");
printf("  ICache              (I) : %s\n", (cp15 & 4096) ? "Enabled" : "Disabled");
printf("  Endian              (B) : %s\n", (cp15 & 128) ? "Big-endian" : "Little-endian");
printf("  System Protection   (S) : %s\n", (cp15 & 256) ? "Enabled" : "Disabled");
printf("  ROM Protection      (R) : %s\n", (cp15 & 512) ? "Enabled" : "Disabled");
printf("  Interrupt Vectors   (V) : %s\n", (cp15 & 8192) ? "0xFFFF0000 - 0xFFFF001C" : "0x00000000 - 0x0000001C");
printf("  Cache Replacement  (RR) : %s\n", (cp15 & 16384) ? "Round Robin" : "Random");
printf("  Set T on PC load   (L4) : %s\n", (cp15 & 32768) ? "Set" : "Don't set");


/*
	CP15 register 2, Translation Table Base Register (TTBR register)s
*/
asm("mrc p15, 0, %0, c2, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C2 Translation Table Base Register : 0x%08X\n", cp15);
printf("  Translation Table Base:%04X\n", (cp15 >> 14) << 14);

/*
	CP15 register 3 Domain Access Control Register

*/
asm("mrc p15, 0, %0, c3, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C3 Domain Access Control Register : 0x%08X\n", cp15);
for (domain = 0; domain < 16; domain++)
	{
	printf("  Domain %02X                 : ", domain);
	decode_domain_register((cp15 >> 30) & 0x03);
	}

/*
	CP15 register 4 is undefined
*/
printf("\n");
printf("CP15-C4 Undefined\n");

/*
	CP15 register 5 Fault Status Registers
*/
asm("mrc p15, 0, %0, c5, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C5 Data Fault Status Register (DFSR) : 0x%08X\n", cp15);
decode_fault_status_register(cp15);

asm("mrc p15, 0, %0, c5, c0, 1" : "=r"(cp15));
printf("\n");
printf("CP15-C5 Instruction Fault Status Register (DFSR) : 0x%08X\n", cp15);
decode_fault_status_register(cp15);

/*
	CP15 register 6 Fault Address Register
*/
asm("mrc p15, 0, %0, c6, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C6 Fault Address Register (FAR) : 0x%08X\n", cp15);

/*
	CP15 register 7 Cache Operations Register
*/
printf("\n");
printf("CP15-C7 Cache Operations Register (cannot be read)\n");

/*
	CP15 register 8 Fault Address Register
*/
printf("\n");
printf("CP15-C8 TLB Operations Register (cannot be read)\n");

/*
	CP15 register 9 Cache Lockdown Register
*/
asm("mrc p15, 0, %0, c9, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C9 DCache Lockdown Register : 0x%08X\n", cp15);
printf("  DCache Way 0 is         : %s\n", (cp15 & 1) ? "Locked" : "Unlocked");
printf("  DCache Way 1 is         : %s\n", (cp15 & 2) ? "Locked" : "Unlocked");
printf("  DCache Way 2 is         : %s\n", (cp15 & 4) ? "Locked" : "Unlocked");
printf("  DCache Way 3 is         : %s\n", (cp15 & 8) ? "Locked" : "Unlocked");

asm("mrc p15, 0, %0, c9, c0, 1" : "=r"(cp15));
printf("\n");
printf("CP15-C9 ICache Lockdown Register : 0x%08X\n", cp15);
printf("  ICache Way 0 is         : %s\n", (cp15 & 1) ? "Locked" : "Unlocked");
printf("  ICache Way 1 is         : %s\n", (cp15 & 2) ? "Locked" : "Unlocked");
printf("  ICache Way 2 is         : %s\n", (cp15 & 4) ? "Locked" : "Unlocked");
printf("  ICache Way 3 is         : %s\n", (cp15 & 8) ? "Locked" : "Unlocked");


/*
	the following code doesn't work on QEMU!
*/
#ifdef NEVER
/*
	CP15 register 9 TCM Region Register
*/
asm("mrc p15, 0, %0, c9, c1, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C9 Data TCM Region Register : 0x%08X\n", cp15);
decode_p15_TCM_size(cp15);

asm("mrc p15, 0, %0, c9, c1, 1" : "=r"(cp15));
printf("\n");
printf("CP15-C9 Instruction TCM Region Register : 0x%08X\n", cp15);
decode_p15_TCM_size(cp15);
#endif
/*
	CP15 register 10 TLB Lockdown Register
*/
asm("mrc p15, 0, %0, c10, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C10 TLB Lockdown Register : 0x%08X\n", cp15);
printf("  Go into %s part of TLB\n", (cp15 & 0x01) ? "lockdown" : "set associative");
printf("  Victim                  : 0x%02X\n", (cp15 >> 26) & 0x07);

/*
	CP15 register 11
*/
printf("\n");
printf("CP15-C11 cannot be read\n");

/*
	CP15 register 12
*/
printf("\n");
printf("CP15-C12 cannot be read\n");

/*
	CP15 register 13 Process ID Register
*/
asm("mrc p15, 0, %0, c13, c0, 0" : "=r"(cp15));
printf("\n");
printf("CP15-C13 Fast Context Switch Extension (FCSE) Process Identifier (PID) : 0x%08X\n", cp15);
printf("  Current process ID      : 0x%02X\n", cp15 >> 25);

/*
	CP15 register 13 Context ID Register
*/
asm("mrc p15, 0, %0, c13, c0, 1" : "=r"(cp15));
printf("\n");
printf("CP15-C13 Context ID Register : 0x%08X\n", cp15);

/*
	CP15 register 14
*/
printf("\n");
printf("CP15-C14 cannot be read\n");

/*
	CP15 register 15 Test and debug register
*/
printf("\n");
printf("CP15-C15 is the Test and Debug Register\n");
}


/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_cpu::text_render();

return 0;
}










