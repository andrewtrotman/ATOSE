/*
	FILE_SYSTEM.H
	-------------
*/

#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

/*
	class ATOSE_FILE_SYSTEM
	-----------------------
*/
class ATOSE_file_system
{
public:
	ATOSE_file_system() {}

	virtual ATOSE_file_control_block *open(uint8_t *filename) = 0;
	virtual ATOSE_file_control_block *close(ATOSE_file_control_block *fcb) = 0;
};

#endif
