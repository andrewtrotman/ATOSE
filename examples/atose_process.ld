ENTRY(_Reset)

STARTUP (obj\atose_process_entry_point.o)

MEMORY
{
operating_system : ORIGIN = 0x0000, LENGTH = 1M
ram : ORIGIN = 0x100000, LENGTH = 16M
}

SECTIONS
{
. = 0x00000000;
. = ALIGN(4);

 __exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > ram
__exidx_end = .;

.text : { *(.text) } > ram
.data : { *(.data) } > ram
.bss : { *(.bss) } > ram

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
	} > ram
}
