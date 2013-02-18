/*
	KERNEL_MALLOC.C
	---------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	FIX: this file should be depricated soon
*/
#include <stdint.h>
#include <stddef.h>

/*
	Get a pointer to the bottom of the heap (and grow from there)
*/
extern uint32_t ATOSE_start_of_heap;
char *external_memory_head = (char *)&ATOSE_start_of_heap;

/*
	ATOSE_KERNEL_MALLOC()
	---------------------
	Allocate "size" bytes of memory alliged on an "alignment" boundry
	Note that this keeps the blocks in the on-chip RAM in the i.MX6Q
	as there is pleanty.

	At present there is no way to give this memory back - because we must avoid
	fragmentation within the kernel.
*/
void *ATOSE_kernel_malloc(size_t size, size_t alignment)
{
void *result;
uint32_t realignment;

/*
	Align the beginning to the requested alignment
*/
realignment = alignment - ((uint32_t) external_memory_head % alignment);

if (realignment > 0 && realignment < alignment)
	external_memory_head += realignment;

/*
	Now return the current pointer and move on
*/
result = external_memory_head;
external_memory_head += size;

return result;
}
