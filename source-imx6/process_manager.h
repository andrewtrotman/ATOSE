/*
	PROCESS_MANAGER.H
	-----------------
*/
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "mmu.h"
#include "process.h"
#include "process_allocator.h"

/*
	class ATOSE_PROCESS_MANAGER
	---------------------------
*/
class ATOSE_process_manager
{
public:
	enum {SUCCESS = 0, ELF_BAD_FROM_START, ELF_BAD_ID, ELF_BAD_BITNESS, ELF_BAD_ENDIAN, ELF_BAD_VERSION, ELF_BAD_HEADER, ELF_BAD_HEADER_VERSION, ELF_BAD_TYPE, ELF_BAD_ARCHITECTURE, ELF_BAD_PROGRAM_HEADER, ELF_BAD_PROGRAM_HEADER_SIZE, ELF_BAD_PROGRAM_HEADER_LOCATION, ELF_BAD_PROGRAM_SEGMENT_TYPE, ELF_BAD_PROGRAM_SEGMENT_SIZE, ELF_BAD_PROGRAM_SEGMENT_LOCATION, ELF_BAD_OUT_OF_PAGES, ELF_BAD_ADDRESS_SPACE_FAIL, ELF_BAD_OUT_OF_PROCESSES};

private:
	ATOSE_mmu *mmu;
	ATOSE_address_space *system_address_space;			// this is the unprotected system "linear flat address space" that can see everything
	ATOSE_process *active_head;			// head of the active process list
	ATOSE_process *active_tail;			// tail of the active process list
	ATOSE_process *current_process;		// the process that is currently executing

protected:
	/*
		Method to create a process from and ELF file
	*/
	uint32_t elf_load(ATOSE_process *process, const uint8_t *file, uint32_t length);

	/*
		Set up a process ready to run
	*/
	uint32_t initialise_process(ATOSE_process *process, size_t entry_point, uint32_t mode);

public:
	ATOSE_process_manager(ATOSE_mmu *mmu, ATOSE_process_allocator *process_allocator);

	/*
		Methods to add and remove from the scheduler's process queue
	*/
 	void push(ATOSE_process *process);
	ATOSE_process *pull(void);

	/*
		Process Management Methods
	*/
	uint32_t create_process(const uint8_t *elf_file, uint32_t length);
	uint32_t create_system_thread(uint32_t (*start)(void));

	uint32_t context_switch(ATOSE_registers *registers);

	ATOSE_process *get_current_process(void) { return current_process; }
	void set_current_process(ATOSE_process *process) { current_process = process; }
	ATOSE_process *get_next_process(void);
} ;

#endif /* PROCESS_MANAGER_H_ */
