/*
	KERNEL_MEMORY_ALLOCATOR.H
	-------------------------
*/
#ifndef KERNEL_MEMORY_ALLOCATOR_H_
#define KERNEL_MEMORY_ALLOCATOR_H_

#include <stdint.h>

extern uint32_t ATOSE_start_of_heap, ATOSE_end_of_heap;


/*
	class ATOSE_KERNEL_MEMORY_ALLOCATOR
	-----------------------------------
*/
class ATOSE_kernel_memory_allocator
{
private:
	uint8_t *base_address;		// start of the kernel heap (lowqest possible memory location that can be used)
	uint8_t *current_address;	// current position in the kernel heap
	uint8_t *top_of_memory;		// end of the kernel heap (highest possible memory location that can be used)

public:
	ATOSE_kernel_memory_allocator()
	{
	current_address = base_address = (uint8_t*) &ATOSE_start_of_heap;
	top_of_memory = (uint8_t*) &ATOSE_end_of_heap;
	}

	void *malloc(uint32_t bytes);
};

#endif /* KERNEL_MEMORY_ALLOCATOR_H_ */
