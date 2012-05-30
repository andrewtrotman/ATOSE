ENTRY(_Reset)

SECTIONS
{
. = 0x10000;
.startup . : { startup.o(.text) }

 __exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
__exidx_end = .;

.text : { *(.text) }
.data : { *(.data) }
.bss : { *(.bss) }

.rodata ALIGN (0x1000) :
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

. = . + 0x1000; /* 4kB of stack memory */
stack_top = .;
ATOSE_top_of_memory = .;
}
