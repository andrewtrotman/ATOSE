/*
	FILE_SYSTEM.C
	-------------
*/
#include <stdint.h>
#include "ascii_str.h"
#include "file_system.h"
#include "file_control_block.h"

/*
	------------------
	------------------
	------------------
*/
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
/*
	------------------
	------------------
	------------------
*/

template <class T> T min(T a, T b) { return a < b ? a : b; }

/*
	ATOSE_FILE_SYSTEM::READ()
	-------------------------
*/
uint64_t ATOSE_file_system::read(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t bytes)
{
uint64_t first_block, last_block, current_block, needed;
uint64_t bytes_read = 0;

/*
	To make sure we don't read past the end of the file we compute how many bytes we really need
*/
bytes = bytes + fcb->file_offset > fcb->file_size_in_bytes ? fcb->file_size_in_bytes - fcb->file_offset : bytes;

/*
	First we copy out of the block already in the buffers
*/
needed = min(bytes, fcb->block_size_in_bytes - (fcb->file_offset % fcb->block_size_in_bytes));
if (needed != 0)
	{
	debug_print_string("GET CURRENT BLOCK\r\n");
	if (fcb->file_system->get_current_block(fcb) == 0)
		return 0;
	memcpy(buffer, fcb->buffer + (fcb->file_offset % fcb->block_size_in_bytes), needed);
	buffer += needed;
	bytes_read += needed;
	debug_print_this("Bytes taken from first block:", needed);
	}

/*
	Read as many blocks as we need to
*/
first_block = (fcb->file_offset / fcb->block_size_in_bytes) + 1;		// +1 because we've done the first block already
last_block = (fcb->file_offset  + bytes) / fcb->block_size_in_bytes;

debug_print_this("FIRST BLOCK:", first_block - 1);		// the actual first block
debug_print_this("LAST BLOCK :", last_block);

if (first_block != last_block)
	{
debug_print_string("GET MORE BLOCKS\r\n");
	/*
		Copy as many full blocks as we need
	*/
	for (current_block = first_block; current_block < last_block; current_block++)
		{
debug_print_string("FETCH\r\n");

		if (fcb->file_system->get_next_block(fcb) == NULL)
			return bytes_read;

		memcpy(buffer, fcb->buffer, fcb->block_size_in_bytes);
		bytes_read += fcb->block_size_in_bytes;
		buffer += fcb->block_size_in_bytes;
debug_print_this("Bytes taken from next block:", fcb->block_size_in_bytes);
		}

	/*
		Get the last block and copy out of that
	*/
	needed = bytes - bytes_read;
	if (needed != 0)
		{
debug_print_string("GET LAST BLOCK\r\n");
		if (fcb->file_system->get_next_block(fcb) == NULL)
			return bytes_read;
		memcpy(buffer, fcb->buffer, bytes - bytes_read);
debug_print_this("Bytes taken from last block:", bytes - bytes_read);
		bytes_read += bytes - bytes_read;
		}
	}

fcb->file_offset += bytes_read;

debug_print_string("DONE\r\n");

return bytes_read;
}
