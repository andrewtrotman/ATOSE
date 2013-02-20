/*
	PROCESS_ALLOCATOR.H
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/

#ifndef PROCESS_ALLOCATOR_H_
#define PROCESS_ALLOCATOR_H_

#include <stdint.h>

#include "process.h"

/*
	class ATOSE_PROCESS_ALLOCATOR
	-----------------------------
*/
class ATOSE_process_allocator
{
private:
	static const uint32_t MAX_PROCESSES = 10;

private:
	uint8_t process_list_memory[MAX_PROCESSES * sizeof(ATOSE_process)];			// this is where the process_list is put because we use the placement new operator
	ATOSE_process *process_list;
	ATOSE_process *top_of_free_list;

public:
	ATOSE_process_allocator(ATOSE_mmu *mmu);

	ATOSE_process *malloc();
	void free(ATOSE_process *process);
} ;

#endif
