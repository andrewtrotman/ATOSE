/*
	PROCESS_MANAGER.H
	-----------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "mmu.h"
#include "process.h"
#include "process_allocator.h"
#include "sleep_wake.h"

class ATOSE_file_control_block;
class ATOSE_FILE;

/*
	class ATOSE_PROCESS_MANAGER
	---------------------------
*/
class ATOSE_process_manager
{
public:
	enum {SUCCESS = 0, ELF_BAD_FROM_START, ELF_BAD_ID, ELF_BAD_BITNESS, ELF_BAD_ENDIAN, ELF_BAD_VERSION, ELF_BAD_HEADER, ELF_BAD_HEADER_VERSION, ELF_BAD_TYPE, ELF_BAD_ARCHITECTURE, ELF_BAD_PROGRAM_HEADER, ELF_BAD_PROGRAM_HEADER_SIZE, ELF_BAD_PROGRAM_HEADER_LOCATION, ELF_BAD_PROGRAM_SEGMENT_TYPE, ELF_BAD_PROGRAM_SEGMENT_SIZE, ELF_BAD_PROGRAM_SEGMENT_LOCATION, ELF_BAD_OUT_OF_PAGES, ELF_BAD_ADDRESS_SPACE_FAIL, ELF_BAD_OUT_OF_PROCESSES, ELF_BAD_FILE_NOT_FOUND };

private:
	ATOSE_mmu *mmu;
	ATOSE_address_space *system_address_space;			// this is the unprotected system "linear flat address space" that can see everything
	ATOSE_process *active_head;			// head of the active process list
	ATOSE_process *active_tail;			// tail of the active process list
	ATOSE_process *idle;						// the idle process
	ATOSE_process *current_process;		// the process that is currently executing
	ATOSE_sleep_wake inactive_queue;	// the sleeping processes

protected:
	/*
		Method to create a process from and ELF file
	*/
	uint32_t elf_load(ATOSE_FILE *infile);

	/*
		Set up a process ready to run
	*/
	uint32_t initialise_process(ATOSE_process *process, size_t entry_point, uint32_t mode, uint32_t top_of_stack);

	/*
		set the idle process
	*/
	void set_idle(ATOSE_process *process) { idle = process; }

	static uint32_t exec(uint32_t parameter);

public:
	ATOSE_process_manager() {}
	void initialise(ATOSE_mmu *mmu, ATOSE_process_allocator *process_allocator);

	/*
		Methods to add and remove from the scheduler's process queue
	*/
 	void push(ATOSE_process *process);
	ATOSE_process *pull(void);
	ATOSE_process *wake(uint32_t id) { return inactive_queue.wake(id); }
	uint32_t sleep_current_process(void) { return inactive_queue.sleep_current_process(); }

	/*
		Process Management Methods
	*/
	uint32_t create_process(const uint8_t *elf_filename);
	uint32_t create_thread(uint32_t (*start)(void));
	uint32_t create_system_thread(uint32_t (*start)(void), const char *name, uint32_t is_idle_process = false);
	uint32_t terminate_current_process(void);

	uint32_t context_switch(ATOSE_registers *registers);

	ATOSE_process *get_current_process(void) { return current_process; }
	void set_current_process(ATOSE_process *process) { current_process = process; }
	ATOSE_process *get_next_process(void);

	void *physical_address_of(void *user_address) { return current_process->address_space->physical_address_of(user_address); }
} ;

#endif /* PROCESS_MANAGER_H_ */
