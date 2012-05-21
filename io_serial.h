/*
	IO_SERIAL.H
	-----------
*/
#ifndef IO_SERIAL_H_
#define IO_SERIAL_H_

#include "io.h"

/*
	class ATOSE_IO_SERIAL
	---------------------
*/
class ATOSE_IO_serial : public ATOSE_IO
{

public:
	ATOSE_IO_serial();
	virtual ~ATOSE_IO_serial() {};

	virtual int read(char *buffer, int bytes);
	virtual int write(const char *buffer, int bytes);
} ;

#endif /* IO_SERIAL_H_ */
