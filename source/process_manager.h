/*
	PROCESS_MANAGER.H
	-----------------
*/
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "io.h"
#include "process.h"
#include "mmu.h"

/*
	class ATOSE_PROCESS_MANAGER
	---------------------------
*/
class ATOSE_process_manager : public ATOSE_IO		/* we're derived from an I/O object so that calls to this object can be made from user space */
{
public:
	enum {SUCCESS = 0, ELF_BAD_FROM_START, ELF_BAD_ID, ELF_BAD_BITNESS, ELF_BAD_ENDIAN, ELF_BAD_VERSION, ELF_BAD_HEADER, ELF_BAD_HEADER_VERSION, ELF_BAD_TYPE, ELF_BAD_ARCHITECTURE, ELF_BAD_PROGRAM_HEADER, ELF_BAD_PROGRAM_HEADER_SIZE, ELF_BAD_PROGRAM_HEADER_LOCATION, ELF_BAD_PROGRAM_SEGMENT_TYPE, ELF_BAD_PROGRAM_SEGMENT_SIZE, ELF_BAD_PROGRAM_SEGMENT_LOCATION, ELF_BAD_OUT_OF_PAGES, ELF_BAD_ADDRESS_SPACE_FAIL};

private:
	ATOSE_process active_process;		// DELETE THIS WHEN WE ADD A SCHEDULER
	ATOSE_mmu *mmu;
	ATOSE_process idle;					// the idle process always exists
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
	uint32_t initialise_process(ATOSE_process *process, size_t entry_point);

	/*
		Methods to add and remove from the scheduler's process queue
	*/
 	void push(ATOSE_process *process);
	ATOSE_process *pull(void);

public:
	ATOSE_process_manager(ATOSE_mmu *mmu);

	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes) { return create_process(buffer, bytes); }

	/*
		Include these in case the user tries to call them
	*/
	virtual uint32_t read_byte(uint8_t *buffer) { return 0; }
	virtual uint32_t write_byte(const uint8_t buffer) { return 0; }
	virtual uint32_t read(uint8_t *buffer, uint32_t bytes) { return 0; }

	/*
		Process Management Methods
	*/
	uint32_t create_process(const uint8_t *elf_file, uint32_t length);
	uint32_t create_idle_process(int (*start)(void));

	ATOSE_process *get_current_process(void) { return current_process; }
	ATOSE_process *get_next_process(void);
} ;

#endif /* PROCESS_MANAGER_H_ */
