/*
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Simple linker script for the i.MX6Q
*/
ENTRY(_Reset)

MEMORY
{
internal_ram : ORIGIN = 0x00901000, LENGTH = 254K		/* the first 1KB for entry stuff; the last 1KB for the i.MX6Q ROM interrupt vector chaining. */
external_ram : ORIGIN = 0x10000000, LENGTH = 1024M
}

SECTIONS
{
. = 0x900000;
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
