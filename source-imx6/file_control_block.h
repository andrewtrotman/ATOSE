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
} ;

#endif
