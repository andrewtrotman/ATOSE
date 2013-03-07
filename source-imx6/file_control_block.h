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

	/*
		Stuff about our current access to the file
	*/
	uint64_t current_block;	// used for sequential access

public:
	ATOSE_file_control_block() {}
} ;

#endif
