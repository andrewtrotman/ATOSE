/*
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Linker script for the Freescale i.MX6Q
*/
ENTRY(_Reset)

MEMORY
{
/*
	i.MX6Q Memory MAP
	0x900000: Start of internal (on-chip) RAM
		Reserved (29KB)
	0x907000
		OCRAM Free Area (196KB)
	0x938000
		MMU Table (24KB)
	0x93E000
		Stack (8120 bytes)
	0x93FFB8
		RAM Exception Vector
	0x93FFFF: End of on-chip RAM

	0x10000000: Start of external (off-chip) RAM
	0xFFFFFFFF: End of externakl (off-chip) RAM

	Off-chip RAM will end before the top of memory if there is less than 3840MB installed

	The on-chip RAM is problematic beause the GNU Linker has problems.  If we set the start address to
	other than on 32K boundary it rounds down to the wrong address.  So we can't start at 0x907000
	we must start a little higher in memory.  As that cuts into the 196KB we had (by 4K) we only get
	192KB to play with,

	For the memory map see:
		on-chip RAM: page 397 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
		off-chip RAM: chapter 2 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
internal_ram : ORIGIN = 0x00908000, LENGTH = 192K
external_ram : ORIGIN = 0x10000000, LENGTH = 1024M
}

SECTIONS
{
. = 0x00908000;
. = ALIGN(4);

.text : { *(.text) } > internal_ram
.data : { *(.data) } > internal_ram
.bss : { *(.bss) } > internal_ram
.rodata : { *(.rodata*) } > internal_ram

. = ALIGN(4);
. = . + 0x1000; /* 4kB of stack memory */
ATOSE_stack_top = .;
ATOSE_start_of_heap = .;
}
