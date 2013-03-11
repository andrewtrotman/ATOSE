/*
	FILE_SYSTEM.C
	-------------
*/
#include <stdint.h>
#include "ascii_str.h"
#include "file_system.h"
#include "file_control_block.h"

template <class T> T min(T a, T b) { return a < b ? a : b; }

/*
	------------------------
	------------------------
	------------------------
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_hex(int data);
/*
	------------------------
	------------------------
	------------------------
*/


/*
	ATOSE_FILE_SYSTEM::READ()
	-------------------------
*/
uint64_t ATOSE_file_system::read(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t wanted)
{
uint64_t bytes_remaining_in_sector, bytes, bytes_read = 0;

/*
	Make sure we don't read past EOF
*/
if (fcb->file_offset >= fcb->file_size_in_bytes)
	return 0;

/*
	To make sure we don't read past the end of the file we compute how many bytes we really need (up to EOF)
*/
wanted = wanted + fcb->file_offset > fcb->file_size_in_bytes ? fcb->file_size_in_bytes - fcb->file_offset : wanted;
if (wanted <= 0)
	return 0;

/*
	Get the first block
*/
if (get_random_block(fcb) == 0)
	return 0;

/*
	The number of bytes we want is the smaller of the bytes remaining in the sector and that which was asked for
	remember that the min in the sector might be cut short because its the last sector in the file.
*/
bytes_remaining_in_sector = min (fcb->file_size_in_bytes - fcb->file_offset, fcb->block_size_in_bytes - (fcb->file_offset % fcb->block_size_in_bytes));
bytes = min(wanted, bytes_remaining_in_sector);

/*
	Copy bytes from the first sector
*/
memcpy(buffer, fcb->buffer + (fcb->file_offset % fcb->block_size_in_bytes), bytes);
buffer += bytes;
fcb->file_offset += bytes;

/*
	If we're at EOF the we're done
*/
if (fcb->file_offset >= fcb->file_size_in_bytes)
	return bytes;

/*
	If we have the number asked for then we're done
*/
if (bytes >= wanted)
	{
	/*
		If the last byte we read was at the end of the block we need to prep the next block
	*/
	if ((fcb->file_offset % fcb->block_size_in_bytes) == 0)
		get_next_block(fcb);

	return bytes;
	}

/*
	How much more do we need? How much have we done?
*/
wanted -= bytes;
bytes_read += bytes;

/*
	read and copy whole blocks for as long as we can
*/
do
	{
	if (get_next_block(fcb) == 0)
		return bytes_read;

	if (wanted < fcb->block_size_in_bytes)
		break;

	memcpy(buffer, fcb->buffer, fcb->block_size_in_bytes);
	buffer += fcb->block_size_in_bytes;
	bytes_read += fcb->block_size_in_bytes;
	fcb->file_offset += fcb->block_size_in_bytes;
	wanted -= fcb->block_size_in_bytes;
	}
while (1);

/*
	Are we at the end of the file yet?
*/
if (wanted <= 0)
	return bytes_read;

/*
	Finally, copy out of the last sector (that has already been loaded)
*/
memcpy(buffer, fcb->buffer, wanted);
bytes_read += wanted;
fcb->file_offset += bytes_read;

return bytes_read;
}

/*
	ATOSE_FILE_SYSTEM::SEEK()
	-------------------------
*/
uint64_t ATOSE_file_system::seek(ATOSE_file_control_block *fcb, uint64_t position_in_file)
{
fcb->file_offset = position_in_file;

return position_in_file;
}

/*
	ATOSE_FILE_SYSTEM::TELL()
	-------------------------
*/
uint64_t ATOSE_file_system::tell(ATOSE_file_control_block *fcb)
{
return fcb->file_offset;
}

/*
	ATOSE_FILE_SYSTEM::WRITE()
	--------------------------
*/
uint64_t ATOSE_file_system::write(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t bytes_to_write)
{
uint64_t total;
uint64_t first_block;
uint64_t current_length_in_blocks;
uint64_t new_length_in_blocks;

/*
	Writing 0 bytes is easy
*/
if (bytes_to_write == 0)
	return 0;

/*
	Check to see if we're going to end up making the file longer and allocate space for it if we can
*/
if (fcb->file_offset + bytes_to_write > fcb->file_size_in_bytes)
	{
	/*
		See how many new blocks we need to add to the file and add them
		Note, a user can seek() to a long distance from the start of the file and then write() and so we
		must determine how many new blocks to create between the end of the file and the new end of file
		which itself might exceed the file system limits.
	*/
	current_length_in_blocks = fcb->file_size_in_bytes / fcb->block_size_in_bytes;
	new_length_in_blocks = (fcb->file_offset + bytes_to_write) / fcb->block_size_in_bytes;

	if (new_length_in_blocks != current_length_in_blocks)
		extend(fcb, fcb->file_offset + bytes_to_write);

	/*
		Get the new file size correct
	*/
	fcb->file_size_in_bytes = fcb->file_offset + bytes_to_write;
	}

/*
	Get the block we need to write into
*/
if (get_random_block(fcb) == 0)
	return 0;

/*
	Check to see if we're in the middle of the block
*/
if (fcb->file_offset / fcb->block_size_in_bytes == (fcb->file_offset + bytes_to_write) / fcb->block_size_in_bytes)
	{
	memcpy(fcb->buffer + (fcb->file_offset % fcb->block_size_in_bytes), buffer, bytes_to_write);
	if (write_current_block(fcb) == 0)
		return 0;

	fcb->file_offset += bytes_to_write;
	return bytes_to_write;
	}

/*
	We write some number of bytes into the first block
*/

first_block = fcb->block_size_in_bytes - (fcb->file_offset % fcb->block_size_in_bytes);
memcpy(fcb->buffer + (fcb->file_offset % fcb->block_size_in_bytes), buffer, first_block);
if (write_current_block(fcb) == 0)
	return 0;
total = first_block;
fcb->file_offset += total;

/*
	Some number of whole blocks
*/
do
	{
	if (get_next_block(fcb) == 0)
		return total;
	if (total + fcb->block_size_in_bytes > bytes_to_write)
		break;
	memcpy(fcb->buffer, buffer + total, fcb->block_size_in_bytes);
	if (write_current_block(fcb) == 0)
		return total;

	fcb->file_offset += fcb->block_size_in_bytes;
	total += fcb->block_size_in_bytes;
	}
while (1);

/*
	Write into the last block
*/
if (total < bytes_to_write)
	{
	memcpy(fcb->buffer, buffer + total, bytes_to_write - total);

	if (write_current_block(fcb) == 0)
		return total;

	fcb->file_offset += bytes_to_write - total;
	}

return bytes_to_write;
}
