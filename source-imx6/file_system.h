/*
	FILE_SYSTEM.H
	-------------
*/

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

class ATOSE_file_control_block;

/*
	class ATOSE_FILE_SYSTEM
	-----------------------
*/
class ATOSE_file_system
{
public:
	ATOSE_file_system() {}

	/*
		A file system is expected to implement the following methods...
	*/
	virtual ATOSE_file_control_block *open(ATOSE_file_control_block *fcb, uint8_t *filename);
	virtual ATOSE_file_control_block *close(ATOSE_file_control_block *fcb) = 0;

	virtual uint8_t *get_current_block(ATOSE_file_control_block *fcb) = 0;
	virtual uint8_t *get_next_block(ATOSE_file_control_block *fcb) = 0;
	virtual uint8_t *get_random_block(ATOSE_file_control_block *fcb, uint64_t bytes_into_file) = 0;

	uint64_t read(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t bytes);
};

#endif
