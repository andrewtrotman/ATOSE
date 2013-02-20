/*
	PROCESS_ALLOCATOR.C
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <new>
#include "process_allocator.h"

/*
	ATOSE_PROCESS_ALLOCATOR::ATOSE_PROCESS_ALLOCATOR()
	--------------------------------------------------
*/
ATOSE_process_allocator::ATOSE_process_allocator(ATOSE_mmu *mmu)
{
uint32_t current;

/*
	Chain together all the free processes in the list with the "bottom" pointing to NULL
	and top_of_free_list pointing to the "top".  Note that top and bottom are actually upside
	down for convenience.
*/
process_list = (ATOSE_process *)process_list_memory;
top_of_free_list = NULL;
for (current = 0; current < MAX_PROCESSES; current++)
	{
	new (process_list + current) ATOSE_process(mmu);
	process_list[current].next = top_of_free_list;
	top_of_free_list = process_list + current;
	}
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC()
	---------------------------------
*/
ATOSE_process *ATOSE_process_allocator::malloc(void)
{
ATOSE_process *answer;

if ((answer = top_of_free_list) != NULL)
	top_of_free_list = top_of_free_list->next;

return answer;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_process *process)
{
process->next = top_of_free_list;
top_of_free_list = process;
}
