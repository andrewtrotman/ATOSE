ENTRY(_Reset)

STARTUP (atose_process_entry_point.o)

MEMORY
{
ram : ORIGIN = 0x20010000, LENGTH = 16M
}

SECTIONS
{
 __exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > ram
__exidx_end = .;

.text : { *(.text) } > ram
.data : { *(.data) } > ram
.bss : { *(.bss) } > ram

.rodata ALIGN(4):
	{
	start_ctors = .;
	*(SORT(.ctors.*));
	*(.init_array);
	end_ctors = .;

	start_dtors = .;
	*(SORT(.dtors.*));
	end_dtors = .;

	*(.rodata*);
	*(.gnu.linkonce.r*);
	} > ram

end = .;
}

/*
ENTRY(_Reset)

STARTUP (atose_process_entry_point.o)

MEMORY
{
ram : ORIGIN = 0x20010000, LENGTH = 16M
}

SECTIONS
{
. = ALIGN(4);

__exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > ram
__exidx_end = .;

.text :
	{
	*(.text);
	} > ram

. = ALIGN(4);
.data :
	{
	 *(.data);
	} > ram

. = ALIGN(4);
.bss :
	{
	__bss_start__ = .;
	*(.bss);
	__bss_end__ = .;
	} > ram

. = ALIGN(4);
.rodata :
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
	
end = .;
_end = .;
__end__ = .;
}

*/