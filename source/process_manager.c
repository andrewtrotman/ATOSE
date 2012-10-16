/*
	PROCESS_MANAGER.C
	-----------------
*/
#include "atose.h"
#include "process_manager.h"

/*
	ATOSE_PROCESS_MANAGER::EXECUTE()
	--------------------------------
*/
uint32_t ATOSE_process_manager::execute(const uint8_t *buffer, uint32_t length)
{
ATOSE *os = ATOSE::get_global_entry_point();
os->io << "[CALL TO START A NEW PROCESS]\r\n";

return length;
}
