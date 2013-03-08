/*
	FILE_CONTROL_BLOCK.H
	--------------------
*/
#ifndef FILE_CONTROL_BLOCK_H_
#define FILE_CONTROL_BLOCK_H_

/*
	class ATOSE_CLASS FILE_CONTROL_BLOCK
	------------------------------------
*/
class ATOSE_file_control_block
{
public:
	/*
		How we access the file
	*/
	ATOSE_file_system *file_system;

	/*
		Stuff about the file itself
	*/
	uint64_t first_block;
	uint64_t block_size_in_bytes;
	uint64_t file_size_in_bytes;

	/*
		Stuff about our current access to the file
	*/
	uint64_t current_block;				// used for sequential access
	uint8_t *buffer;						// this is the current block (as read from disk)
	uint64_t file_offset;				// current offset within the file (buffer offset is file_offset % block_size_in_bytes)

public:
	ATOSE_file_control_block() {}
} ;

#endif
