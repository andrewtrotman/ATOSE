/*
	PROCESS_MANAGER.C
	-----------------
*/
#include "atose.h"
#include "process_manager.h"

/*
	ATOSE_PROCESS_MANAGER::WRITE()
	------------------------------
*/
uint32_t ATOSE_process_manager::write(const uint8_t *buffer, uint32_t bytes)
{
ATOSE *os = ATOSE::get_global_entry_point();

os->io << "[CALL TO START A NEW PROCESS]\r\n";

return bytes;
}
