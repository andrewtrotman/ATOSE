ENTRY(_Reset)

MEMORY
{
on_chip_ram  : ORIGIN = 0x0000, LENGTH = 32K
off_chip_ram : ORIGIN = 0x40000000, LENGTH = 128M
}

SECTIONS
{
. = 0x00000000;
. = ALIGN(4);
.startup . : { startup.o(.text) } > on_chip_ram

 __exidx_start = .;
.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > on_chip_ram
__exidx_end = .;

.text : { *(.text) } > on_chip_ram
.data : { *(.data) } > on_chip_ram
.bss : { *(.bss) } > on_chip_ram

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
	} > on_chip_ram
. = ALIGN(4);
. = . + 0x1000; /* 4kB of stack memory */
stack_top = .;
ATOSE_start_of_heap = .;
. = . + 0x1000; /* 4kB of heap */
ATOSE_end_of_heap = .;

.off_chip_ram : { *(.off_chip_ram) } > off_chip_ram
}
