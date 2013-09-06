/*
	FILE_CONTROL_BLOCK.H
	--------------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef FILE_CONTROL_BLOCK_H_
#define FILE_CONTROL_BLOCK_H_

#include "file_system.h"

/*
	class ATOSE_FILE_CONTROL_BLOCK
	------------------------------
*/
class ATOSE_file_control_block
{
public:
	static const uint32_t MAX_PATH = 260;			// 260 is the limit on Windows

public:
	/*
		How we access the file
	*/
	ATOSE_file_system *file_system;

	/*
		We need to keep the filename so that we can patch-up the
	*/
	uint8_t filename[MAX_PATH];

	/*
		Stuff about the file itself
	*/
	uint64_t first_block;						// the location, on disk, of the first block of the file (may be 0 for 0-length files)
	uint64_t block_size_in_bytes;				// the size of a block (or a cluster)
	uint64_t file_size_in_bytes;				// length of the file in bytes, 0 for a 0-length file

	/*
		Stuff about our current access to the file
	*/
	uint64_t current_block;						// used for sequential access, the current block of the file
	uint8_t *buffer;								// this is the contents of the current block (as read from disk)
	uint64_t file_offset;						// current offset within the file (buffer offset is file_offset % block_size_in_bytes)

public:
	ATOSE_file_control_block() {}

	uint64_t seek(uint64_t position_in_file) { return file_system->seek(this, position_in_file); }
	uint64_t tell(void) { return file_system->tell(this); }
	uint64_t read(uint8_t *buffer, uint64_t bytes) { return file_system->read(this, buffer, bytes); }
	uint64_t read(void *buffer, uint64_t bytes) { return read((uint8_t *)buffer, bytes); }
	uint64_t write(uint8_t *buffer, uint64_t bytes_to_write) { return file_system->write(this, buffer, bytes_to_write); }
	uint64_t write(void *buffer, uint64_t bytes_to_write) { return write((uint8_t *)buffer, bytes_to_write); }
	ATOSE_file_control_block *close(void) { return file_system->close(this); }
} __attribute__ ((packed));;

#endif
