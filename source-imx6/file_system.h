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
	enum
		{
		SUCCESS = 0,
		ERROR_FILE_SIZE_EXCEEDED,
		ERROR_DISK_FULL,
		ERROR_BAD_FILE
		};
public:
	ATOSE_file_system() {}

	/*
		A file system is expected to implement the following methods...
	*/
	virtual ATOSE_file_control_block *create(ATOSE_file_control_block *fcb, uint8_t *filename) = 0;
	virtual ATOSE_file_control_block *open(ATOSE_file_control_block *fcb, uint8_t *filename) = 0;
	virtual ATOSE_file_control_block *close(ATOSE_file_control_block *fcb) = 0;
	virtual uint64_t seek(ATOSE_file_control_block *fcb, uint64_t position_in_file);
	virtual uint64_t tell(ATOSE_file_control_block *fcb);
	virtual uint64_t read(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t bytes);
	virtual uint64_t write(ATOSE_file_control_block *fcb, uint8_t *buffer, uint64_t bytes_to_write);
	virtual uint64_t extend(ATOSE_file_control_block *fcb, uint64_t new_length) = 0;
	virtual uint32_t unlink(uint8_t *filename) = 0;

	virtual uint8_t *get_random_block(ATOSE_file_control_block *fcb) = 0;
	virtual uint8_t *get_next_block(ATOSE_file_control_block *fcb) = 0;
	virtual uint8_t *write_current_block(ATOSE_file_control_block *fcb) = 0;
};

#endif
