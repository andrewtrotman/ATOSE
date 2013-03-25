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
#include "semaphore.h"
#include "pipe_task.h"

/*
	class ATOSE_PROCESS_ALLOCATOR
	-----------------------------
*/
class ATOSE_process_allocator
{
private:
	static const uint32_t MAX_PROCESSES = 10;
	static const uint32_t MAX_ADDRESS_SPACES = 10;
	static const uint32_t MAX_THREADS = 10;
	static const uint32_t MAX_SEMAPHORES = 10;
	static const uint32_t MAX_PIPE_TASKS = 10;

private:
	ATOSE_process process_list[MAX_PROCESSES];
	ATOSE_process *free_processes_head;

	ATOSE_address_space address_space_list[MAX_ADDRESS_SPACES];
	ATOSE_address_space *free_address_space_head;

	ATOSE_thread thread_list[MAX_THREADS];
	ATOSE_thread *free_thread_head;

	ATOSE_semaphore semaphore_list[MAX_SEMAPHORES];
	ATOSE_semaphore *free_semaphore_head;

	ATOSE_pipe_task pipe_task_list[MAX_PIPE_TASKS];
	ATOSE_pipe_task *free_pipe_task_head;

public:
	ATOSE_process_allocator(ATOSE_mmu *mmu);

	ATOSE_process *malloc(ATOSE_address_space *space = NULL);
	void free(ATOSE_process *process);

	ATOSE_address_space *malloc_address_space(void);
	void free(ATOSE_address_space *space);

	ATOSE_semaphore *malloc_semaphore(void);
	void free(ATOSE_semaphore *semaphore);

	ATOSE_pipe_task *malloc_pipe_task(void);
	void free(ATOSE_pipe_task *task);
} ;

#endif
