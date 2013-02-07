/*
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Simple linker script for the i.MX6Q
*/
ENTRY(_Reset)

MEMORY
{
internal_ram : ORIGIN = 0x900000, LENGTH = 255K		/* the last 1KB is used by the i.MX6Q ROM for (amongst other stuff) the interrupt vector chaining. */
external_ram : ORIGIN = 0x10000000, LENGTH = 1G
}

SECTIONS
{
. = 0x900000;
. = ALIGN(4);

.text : { *(.ATOSE_entry_point); *(.text) } > internal_ram
.data : { *(.data) } > internal_ram
.bss : { *(.bss) } > internal_ram

 __exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > internal_ram
__exidx_end = .;

.rodata ALIGN(4):
	{
	start_ctors = .;
	*(SORT(.ctors.*))
	*(.init_array)
	end_ctors = .;

	start_dtors = .;
	*(SORT(.dtors.*))
	end_dtors = .;

	*(.rodata*)
	*(.gnu.linkonce.r*)
	} > internal_ram
. = ALIGN(4);
. = . + 0x1000; /* 4kB of stack memory */
stack_top = .;
ATOSE_start_of_heap = .;
. = . + 0x1000; /* 4kB of heap */
ATOSE_end_of_heap = .;
}
