/*
	KERNEL_MEMORY_ALLOCATOR.C
	-------------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "kernel_memory_allocator.h"

/*
	ATOSE_KERNEL_MEMORY_ALLOCATOR::ATOSE_KERNEL_MEMORY_ALLOCATOR()
	--------------------------------------------------------------
*/
ATOSE_kernel_memory_allocator::ATOSE_kernel_memory_allocator()
{
set_allocation_page(0, 0);
}

/*
	ATOSE_KERNEL_MEMORY_ALLOCATOR::SET_ALLOCATION_PAGE()
	----------------------------------------------------
	Set the base address and length of the continious extent of memory that can be used
	for memory allocation within the kernel

	new_page_address must be word aligned
*/
void ATOSE_kernel_memory_allocator::set_allocation_page(uint8_t *new_base_address, uint32_t length)
{
current_address = base_address = new_base_address;
top_of_memory = base_address + length;
}

/*
	ATOSE_KERNEL_MEMORY_ALLOCATOR::MALLOC()
	---------------------------------------
	Allocate "size" bytes of memory alliged on an "alignment" boundry

	At present there is no way to give this memory back - because we must avoid
	fragmentation within the kernel.
*/
void *ATOSE_kernel_memory_allocator::malloc(size_t size, size_t alignment)
{
void *result;
uint32_t realignment;

/*
	Align the beginning to the requested alignment
*/
realignment = alignment - ((uint32_t)current_address % alignment);

/*
	check we have the memory available
*/
if (current_address + realignment + size > top_of_memory)
	return NULL;

/*
	Re-align
*/
if (realignment > 0 && realignment < alignment)
	current_address += realignment;

/*
	Now return the current pointer and move on
*/
result = current_address;
current_address += size;

return result;
}
