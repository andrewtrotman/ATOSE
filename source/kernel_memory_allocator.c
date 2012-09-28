/*
	KERNEL_MEMORY_ALLOCATOR.C
	-------------------------
*/
#include "kernel_memory_allocator.h"

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

