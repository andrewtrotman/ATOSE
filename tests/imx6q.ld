ENTRY(_Reset)

SECTIONS
{
. = 0x910080;
. = ALIGN(4);

.text : { *(.ATOSE_entry_point); *(.text) } 
.data : { *(.data) } 
.bss : { *(.bss) } 

__exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } 
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
	} 
. = ALIGN(4);
. = . + 0x1000; /* 4kB of stack memory */
stack_top = .;
ATOSE_start_of_heap = .;
. = . + 0x1000; /* 4kB of heap */
ATOSE_end_of_heap = .;
}
