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
	ATOSE_process_manager() : ATOSE_IO() {}

	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes);

	/*
		Include these in case the user tries to call them
	*/
	virtual uint32_t read_byte(uint8_t *buffer) { return 0; }
	virtual uint32_t write_byte(const uint8_t buffer) { return 0; }
	virtual uint32_t read(uint8_t *buffer, uint32_t bytes) { return 0; }
} ;



#endif /* PROCESS_MANAGER_H_ */
