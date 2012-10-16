/*
	PROCESS_MANAGER.H
	-----------------
*/
#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "io.h"

/*
	class ATOSE_PROCESS_MANAGER
	---------------------------
*/
class ATOSE_process_manager : public ATOSE_IO		/* we're derived from an I/O object so that calls to this object can be made from user space */
{
public:
	enum {SUCCESS = 0, ELF_BAD_FROM_START, ELF_BAD_ID, ELF_BAD_BITNESS, ELF_BAD_ENDIAN, ELF_BAD_VERSION, ELF_BAD_HEADER, ELF_BAD_HEADER_VERSION, ELF_BAD_TYPE, ELF_BAD_ARCHITECTURE, ELF_BAD_PROGRAM_HEADER, ELF_BAD_PROGRAM_HEADER_SIZE, ELF_BAD_PROGRAM_HEADER_LOCATION, ELF_BAD_PROGRAM_SEGMENT_TYPE};

protected:
	uint32_t elf_load(const uint8_t *file, uint32_t length);
	uint32_t execute(const uint8_t *elf_file, uint32_t length);

public:
	ATOSE_process_manager() : ATOSE_IO() {}

	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes) { return execute(buffer, bytes); }

	/*
		Include these in case the user tries to call them
	*/
	virtual uint32_t read_byte(uint8_t *buffer) { return 0; }
	virtual uint32_t write_byte(const uint8_t buffer) { return 0; }
	virtual uint32_t read(uint8_t *buffer, uint32_t bytes) { return 0; }
} ;



#endif /* PROCESS_MANAGER_H_ */
