/*
	PROCESS_ALLOCATOR.C
	-------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include <new>
#include "process_allocator.h"

/*
	ATOSE_PROCESS_ALLOCATOR::INITIALISE()
	-------------------------------------
*/
void ATOSE_process_allocator::initialise(ATOSE_mmu *mmu)
{
uint32_t current;

/*
	chain together each of the process objects
*/
free_processes_head = NULL;
for (current = 0; current < MAX_PROCESSES; current++)
	{
	process_list[current].next = free_processes_head;
	free_processes_head = process_list + current;
	}

/*
	chain together each of the address_space objects
*/
free_address_space_head = NULL;
for (current = 0; current < MAX_ADDRESS_SPACES; current++)
	{
	address_space_list[current].next = free_address_space_head;
	address_space_list[current].initialise(mmu, current);
	free_address_space_head = address_space_list + current;
	}


/*
	chain together each of the semaphore objects
*/
free_semaphore_head = NULL;
for (current = 0; current < MAX_SEMAPHORES; current++)
	{
	semaphore_list[current].clear();
	semaphore_list[current].next = free_semaphore_head;
	free_semaphore_head = semaphore_list + current;
	}

/*
	chain together each of the pipe tasks
*/
free_pipe_task_head = NULL;
for (current = 0; current < MAX_PIPE_TASKS; current++)
	{
	pipe_task_list[current].next = free_pipe_task_head;
	free_pipe_task_head = pipe_task_list + current;
	}
/*
	chain together each of the pipes
*/
free_pipe_head = NULL;
for (current = 0; current < MAX_PIPES; current++)
	{
	pipe_list[current].next = free_pipe_head;
	free_pipe_head = pipe_list + current;
	}
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC_ADDRESS_SPACE()
	-----------------------------------------------
*/
ATOSE_address_space *ATOSE_process_allocator::malloc_address_space(void)
{
ATOSE_address_space *address_space;

if ((address_space = free_address_space_head) != NULL)
	free_address_space_head = free_address_space_head->next;

return address_space;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_address_space *space)
{
/*
	Only destroy the object if the reference count drops to zero
*/
if (space->destroy() == 0)
	{
	space->next = free_address_space_head;
	free_address_space_head = space;
	}
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC()
	---------------------------------
*/
ATOSE_process *ATOSE_process_allocator::malloc(ATOSE_address_space *space)
{
ATOSE_process *process;
ATOSE_address_space *address_space;

/*
	Make sure we have the resources to create a process
*/
if ((process = free_processes_head) == NULL)
	return NULL;

/*
	Create an address space if we need to
*/
if ((address_space = space == NULL ? malloc_address_space() : space) == NULL)
	return NULL;

/*
	We have all the resources so we can now use them
*/
free_processes_head = free_processes_head->next;

/*
	Build a process and return it
*/
process->address_space = address_space;
process->current_pipe_task = NULL;

return process;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_process *process)
{
/*
	We might be done with the address space, or we might not.  If its being used by other processes in the system then we cannot get rid of it.
*/
free(process->address_space);

/*
	We're done with the process object
*/
process->next = free_processes_head;
free_processes_head = process;
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC_SEMAPHORE()
	-------------------------------------------
*/
ATOSE_semaphore *ATOSE_process_allocator::malloc_semaphore(void)
{
ATOSE_semaphore *semaphore;

if ((semaphore = free_semaphore_head) != NULL)
	free_semaphore_head = free_semaphore_head->next;

return semaphore;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_semaphore *semaphore)
{
semaphore->next = free_semaphore_head;
free_semaphore_head = semaphore;
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC_PIPE_TASK()
	-------------------------------------------
*/
ATOSE_pipe_task *ATOSE_process_allocator::malloc_pipe_task(void)
{
ATOSE_pipe_task *task;

if ((task = free_pipe_task_head) != NULL)
	free_pipe_task_head = free_pipe_task_head->next;

return task;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_pipe_task *task)
{
task->next = free_pipe_task_head;
free_pipe_task_head = task;
}

/*
	ATOSE_PROCESS_ALLOCATOR::MALLOC_PIPE()
	--------------------------------------
*/
ATOSE_pipe *ATOSE_process_allocator::malloc_pipe(void)
{
ATOSE_pipe *pipe;

if ((pipe = free_pipe_head) != NULL)
	free_pipe_head = free_pipe_head->next;

return pipe;
}

/*
	ATOSE_PROCESS_ALLOCATOR::FREE()
	-------------------------------
*/
void ATOSE_process_allocator::free(ATOSE_pipe *pipe)
{
pipe->next = free_pipe_head;
free_pipe_head = pipe;
}
