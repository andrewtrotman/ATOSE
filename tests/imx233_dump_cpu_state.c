/*
	DUMP_CPU_STATE.C
	----------------
*/
#include <stdarg.h>
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#include "../source/ascii_str.h"

void __cs3_isr_undef(void){}
void __cs3_isr_pabort(void){}
void __cs3_isr_dabort(void){}
void __cs3_isr_reserved(void){}
void __cs3_isr_fiq(void){}
void ATOSE_isr_irq(void){}
void ATOSE_isr_swi(void){}

/*
	PUTC()
	------
*/
void debug_putc(char ch)
{
int loop = 0;

while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF)
	if (++loop > 10000)
		break;

/* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
HW_UARTDBGDR_WR(ch);
}


/*
	PRINT_STRING()
	--------------
*/
void print_string(char *string)
{
while (*string)
	{
	debug_putc(*string);
	string++;
	}
}

/*
	PRINTF()
	--------
*/
void debug_printf(const char *fmt, ...)
{
va_list args;
int one;
char buffer[32];


va_start(args, fmt);
while (*fmt)
	{
	if (*fmt == '%')
		{
		fmt++;
		switch (*fmt)
			{
			case 'x':
			case 'X':
				ASCII_itoa((uint32_t)va_arg(args, int), buffer, 16);
				print_string(buffer);
				break;
			case 's':
				print_string(va_arg(args, char *));
				break;
			case 'd':
				ASCII_itoa((uint32_t)va_arg(args, int), buffer, 10);
				print_string(buffer);
				break;
			case 'c':
				debug_putc(va_arg(args, int));
				break;
			case '%':
				debug_putc('%');
				break;
			default:
				break;
			}
		}
	else
		debug_putc(*fmt);
	fmt++;
	}
va_end(args);
}

/*
	TEXT_RENDER_DECODE_CACHE_SIZE()
	-------------------------------
*/
void text_render_decode_cache_size(int cache_size)
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

debug_printf("      Size             : %s\n", string);
debug_printf("      Restrictions (P) : %s\n", cache_size >> 11 ? "Some" : "No");
switch (cache_size &0x03)
	{
	case 0: string = "8 bytes"; break;
	case 1: string = "16 bytes"; break;
	case 2: string = "32 bytes"; break;
	case 3: string = "64 bytes"; break;
	}
debug_printf("      Cache lines      : %s\n", string);


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
debug_printf("      Associativity    : %s-way\n", string);
}


/*
	DECODE_DOMAIN_REGISTER()
	------------------------
*/
void decode_domain_register(int value)
{
switch (value & 0x03)
	{
	case 0: debug_printf("No access\n"); break;
	case 1: debug_printf("Client access\n"); break;
	case 2: debug_printf("Reserved\n"); break;
	case 3: debug_printf("Manager\n"); break;
	}
}

/*
	DECODE_FAULT_STATUS_REGISTER()
	------------------------------
*/
void decode_fault_status_register(int cp15)
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
debug_printf("  Fault Type              : %s\n", string);
switch (cp15 & 0x0F)
	{
	case 7:
	case 9:
	case 11:
	case 13:
	case 14:
	case 15:
		debug_printf("  in Domain               : %X\n", (cp15 >> 4) & 0x0F);
		break;
	}
}

/*
	DECODE_P15_TCM_SIZE()
	---------------------
*/
void decode_p15_TCM_size(int value)
{
const char *string;

debug_printf("  Enable bit              : %s\n", (value & 0x01) ? "Enabled" : "Disabled");

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

debug_printf("  Size                    : %s\n", string);
debug_printf("  Address                 : %X\n", value >> 12);
}

/*
	TEXT_RENDER()
	-------------
*/
void text_render(void)
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
debug_printf("CPSR: 0x%X\n", cpsr);
debug_printf("  Negative   (N): %X\n", (cpsr >> 31) & 0x01);
debug_printf("  Zero       (Z): %X\n", (cpsr >> 30) & 0x01);
debug_printf("  Carry      (C): %X\n", (cpsr >> 29) & 0x01);
debug_printf("  Overflow   (V): %X\n", (cpsr >> 28) & 0x01);
debug_printf("  Saturation (Q): %X\n", (cpsr >> 27) & 0x01);
debug_printf("  SIMD >=   (GE): %X\n", (cpsr >> 16) & 0x0F);

debug_printf("  Jazelle    (J): %X\n", j = (cpsr >> 24) & 0x01);
debug_printf("  Thumb      (T): %X\n", t = (cpsr >> 5) & 0x01);
switch ((j << 1) | t)
	{
	case 0x00: debug_printf("    ARM instruction set\n");	break;
	case 0x01: debug_printf("    Thumb instruction set\n");	break;
	case 0x02: debug_printf("    Jazelle (Java) instruction set\n");	break;
	case 0x03: debug_printf("    Unknown instruction set (invalid)\n");	break;
	}

debug_printf("  Endianness (E): %X (%s)\n", (cpsr >> 9) & 0x01, ((cpsr >> 9) & 0x01) ? "Big Endian" : "Little Endian");
debug_printf("  Data abort (A): %X (%s)\n", (cpsr >> 8) & 0x01, ((cpsr >> 8) & 0x01) ? "Disabled" : "Enabled");
debug_printf("  IRQ        (I): %X (%s)\n", (cpsr >> 7) & 0x01, ((cpsr >> 7) & 0x01) ? "Disabled" : "Enabled");
debug_printf("  FIRQ       (F): %X (%s)\n", (cpsr >> 6) & 0x01, ((cpsr >> 6) & 0x01) ? "Disabled" : "Enabled");
debug_printf("  Mode       (M): 0x%X\n", cpsr & 0x1F);

switch (cpsr & 0x1F)
	{
	case 0x10: debug_printf("    USER mode\n");	break;
	case 0x11: debug_printf("    FIRQ mode\n");	break;
	case 0x12: debug_printf("    IRQ mode\n");	break;
	case 0x13: debug_printf("    SUPERVISOR mode\n");	break;
	case 0x17: debug_printf("    ABORT mode\n");	break;
	case 0x1C: debug_printf("    UNDEFINED mode\n");	break;
	case 0x1F: debug_printf("    SYSTEM mode\n");	break;
	default:   debug_printf("    mode is unknown and undefined\n");	break;
	}

/*
	Now deal with the CP15 co-processor (mostly memory management stuff)
	register 0 : ID codes
*/
asm("mrc p15, 0, %0, c0, c0, 0" : "=r"(cp15));

debug_printf("\n");
debug_printf("CP15-C0 ID code : 0x%X\n", cp15);
debug_printf("  Implementer   : 0x%X ('%c')\n",  cp15 >> 24, cp15 >> 24);
if ((cp15 & (1 << 19)) == 0)
	{
	architecture = (cp15 >> 12) & 0x0F;
	if (architecture == 0x00)
		debug_printf("  pre-ARMv4 CPU (obsolete)\n");
	else 
		debug_printf("  ARM V%X\n", architecture);
	}

if (architecture != 0x00 && architecture != 0x07)
	{
	debug_printf("    Variant       : 0x%X\n", (cp15 >> 20) & 0x0F);
	debug_printf("    Architecture  : 0x%X\n", (cp15 >> 16) & 0x0F);
	debug_printf("    Part Number   : 0x%X\n", (cp15 >>  4) & 0xFFF);
	debug_printf("    Revision      : 0x%X\n", cp15 & 0x0F);
	}


/*
	CP15 register 0, cache type register
*/
asm("mrc p15, 0, %0, c0, c0, 1" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C0 Cache type register: 0x%X\n", cp15);
debug_printf("    Ctype              : %X (should be 0x0E)\n", (cp15 >> 25) & 0x0F);
debug_printf("    Seperate Caches (S): %X (caches are %s)\n", (cp15 >> 24) % 0x01, (cp15 >> 24) & 0x01 ? "Seperate (Harvard)" : "Unified");
/*
	Data cache
*/
cache_size = (cp15 >> 12) & 0xFFF;
debug_printf("    Dsize              : 0x%X\n", cache_size);
text_render_decode_cache_size(cache_size);

/*
	Instruction Cache
*/
cache_size = cp15 & 0xFFF;
debug_printf("    Isize              : 0x%X\n", cache_size);
text_render_decode_cache_size(cache_size);

/*
	CP15 register 0, Tighly coupled memory (TCM) type register
*/
asm("mrc p15, 0, %0, c0, c0, 2" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C0 Tightly Coupled Memory (TCM) register: 0x%X\n", cp15);
debug_printf("  Banks of Instruction TCM:%d\n", cp15 & 0x7);
debug_printf("  Banks of Data TCM       :%d\n", (cp15 >> 16) & 0x7);

/*
	CP15 register 1, Control Register
*/
asm("mrc p15, 0, %0, c1, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C1 Control Register  : 0x%X\n", cp15);
debug_printf("  MMU                 (M) : %s\n", (cp15 & 1) ? "Enabled" : "Disabled");
debug_printf("  Alignment faults    (A) : %s\n", (cp15 & 2) ? "Enabled" : "Disabled");
debug_printf("  DCache              (C) : %s\n", (cp15 & 4) ? "Enabled" : "Disabled");
debug_printf("  ICache              (I) : %s\n", (cp15 & 4096) ? "Enabled" : "Disabled");
debug_printf("  Endian              (B) : %s\n", (cp15 & 128) ? "Big-endian" : "Little-endian");
debug_printf("  System Protection   (S) : %s\n", (cp15 & 256) ? "Enabled" : "Disabled");
debug_printf("  ROM Protection      (R) : %s\n", (cp15 & 512) ? "Enabled" : "Disabled");
debug_printf("  Interrupt Vectors   (V) : %s\n", (cp15 & 8192) ? "0xFFFF0000 - 0xFFFF001C" : "0x00000000 - 0x0000001C");
debug_printf("  Cache Replacement  (RR) : %s\n", (cp15 & 16384) ? "Round Robin" : "Random");
debug_printf("  Set T on PC load   (L4) : %s\n", (cp15 & 32768) ? "Set" : "Don't set");


/*
	CP15 register 2, Translation Table Base Register (TTBR register)s
*/
asm("mrc p15, 0, %0, c2, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C2 Translation Table Base Register : 0x%X\n", cp15);
debug_printf("  Translation Table Base:%X\n", (cp15 >> 14) << 14);

/*
	CP15 register 3 Domain Access Control Register

*/
asm("mrc p15, 0, %0, c3, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C3 Domain Access Control Register : 0x%X\n", cp15);
for (domain = 0; domain < 16; domain++)
	{
	debug_printf("  Domain %X                 : ", domain);
	decode_domain_register((cp15 >> 30) & 0x03);
	}

/*
	CP15 register 4 is undefined
*/
debug_printf("\n");
debug_printf("CP15-C4 Undefined\n");

/*
	CP15 register 5 Fault Status Registers
*/
asm("mrc p15, 0, %0, c5, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C5 Data Fault Status Register (DFSR) : 0x%X\n", cp15);
decode_fault_status_register(cp15);

asm("mrc p15, 0, %0, c5, c0, 1" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C5 Instruction Fault Status Register (DFSR) : 0x%X\n", cp15);
decode_fault_status_register(cp15);

/*
	CP15 register 6 Fault Address Register
*/
asm("mrc p15, 0, %0, c6, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C6 Fault Address Register (FAR) : 0x%X\n", cp15);

/*
	CP15 register 7 Cache Operations Register
*/
debug_printf("\n");
debug_printf("CP15-C7 Cache Operations Register (cannot be read)\n");

/*
	CP15 register 8 Fault Address Register
*/
debug_printf("\n");
debug_printf("CP15-C8 TLB Operations Register (cannot be read)\n");

/*
	CP15 register 9 Cache Lockdown Register
*/
asm("mrc p15, 0, %0, c9, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C9 DCache Lockdown Register : 0x%X\n", cp15);
debug_printf("  DCache Way 0 is         : %s\n", (cp15 & 1) ? "Locked" : "Unlocked");
debug_printf("  DCache Way 1 is         : %s\n", (cp15 & 2) ? "Locked" : "Unlocked");
debug_printf("  DCache Way 2 is         : %s\n", (cp15 & 4) ? "Locked" : "Unlocked");
debug_printf("  DCache Way 3 is         : %s\n", (cp15 & 8) ? "Locked" : "Unlocked");

asm("mrc p15, 0, %0, c9, c0, 1" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C9 ICache Lockdown Register : 0x%X\n", cp15);
debug_printf("  ICache Way 0 is         : %s\n", (cp15 & 1) ? "Locked" : "Unlocked");
debug_printf("  ICache Way 1 is         : %s\n", (cp15 & 2) ? "Locked" : "Unlocked");
debug_printf("  ICache Way 2 is         : %s\n", (cp15 & 4) ? "Locked" : "Unlocked");
debug_printf("  ICache Way 3 is         : %s\n", (cp15 & 8) ? "Locked" : "Unlocked");


/*
	the following code doesn't work on QEMU!
*/
#ifdef NEVER
/*
	CP15 register 9 TCM Region Register
*/
asm("mrc p15, 0, %0, c9, c1, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C9 Data TCM Region Register : 0x%X\n", cp15);
decode_p15_TCM_size(cp15);

asm("mrc p15, 0, %0, c9, c1, 1" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C9 Instruction TCM Region Register : 0x%X\n", cp15);
decode_p15_TCM_size(cp15);
#endif
/*
	CP15 register 10 TLB Lockdown Register
*/
asm("mrc p15, 0, %0, c10, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C10 TLB Lockdown Register : 0x%X\n", cp15);
debug_printf("  Go into %s part of TLB\n", (cp15 & 0x01) ? "lockdown" : "set associative");
debug_printf("  Victim                  : 0x%X\n", (cp15 >> 26) & 0x07);

/*
	CP15 register 11
*/
debug_printf("\n");
debug_printf("CP15-C11 cannot be read\n");

/*
	CP15 register 12
*/
debug_printf("\n");
debug_printf("CP15-C12 cannot be read\n");

/*
	CP15 register 13 Process ID Register
*/
asm("mrc p15, 0, %0, c13, c0, 0" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C13 Fast Context Switch Extension (FCSE) Process Identifier (PID) : 0x%X\n", cp15);
debug_printf("  Current process ID      : 0x%X\n", cp15 >> 25);

/*
	CP15 register 13 Context ID Register
*/
asm("mrc p15, 0, %0, c13, c0, 1" : "=r"(cp15));
debug_printf("\n");
debug_printf("CP15-C13 Context ID Register : 0x%X\n", cp15);

/*
	CP15 register 14
*/
debug_printf("\n");
debug_printf("CP15-C14 cannot be read\n");

/*
	CP15 register 15 Test and debug register
*/
debug_printf("\n");
debug_printf("CP15-C15 is the Test and Debug Register\n");
}


/*
	C_ENTRY()
	---------
*/
int c_entry(void)
{
text_render();

return 0;
}
