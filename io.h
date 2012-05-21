/*
	IO.H
	----
*/
#ifndef IO_H_
#define IO_H_

#include "ascii_str.h"

/*
	class ATOSE_IO
	--------------
*/

class ATOSE_IO
{
public:
	ATOSE_IO() {}
	virtual ~ATOSE_IO() {}

	virtual int read_byte(char buffer) { return read(&buffer, 1); }
	virtual int write_byte(const char buffer) { return write(&buffer, 1); }

	virtual int read(char *buffer, int bytes) = 0;
	virtual int write(const char *buffer, int bytes) = 0;

	virtual int puts(const char *message) { write(message, ASCII_strlen(message)); write("\n", 1); }
} ;

#endif /* IO_H_ */


