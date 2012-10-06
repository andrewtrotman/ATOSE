/*
	KERNEL_MEMORY_ALLOCATOR.C
	-------------------------
*/
#include "kernel_memory_allocator.h"

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
*/
void *ATOSE_kernel_memory_allocator::malloc(uint32_t bytes)
{
uint32_t extra_bytes[] = {0, 3, 2, 1};
uint8_t *answer;

/*
	ARM has alignment requirements so round up to the nearest number of whole words
*/
bytes += extra_bytes[(bytes & 0x03)];

/*
	Make sure we have room
*/
if (current_address + bytes > top_of_memory)
	{
	/*
		Compute the new top of heap and return the old one
	*/
	answer = current_address;
	current_address += bytes;
	return answer;
	}

/*
	No room so we return NULL
*/
return 0;
}

