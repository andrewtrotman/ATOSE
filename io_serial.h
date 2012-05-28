/*
	IO_SERIAL.H
	-----------
*/
#ifndef IO_SERIAL_H_
#define IO_SERIAL_H_

#include "io.h"
#include "circular_buffer.h"

/*
	class ATOSE_IO_SERIAL
	---------------------
*/
class ATOSE_IO_serial : public ATOSE_IO
{
private:
	ATOSE_circular_buffer<unsigned char, 1024> buffer;

protected:
	void push(unsigned char *byte);

public:
	ATOSE_IO_serial();
	virtual ~ATOSE_IO_serial();

	virtual int read(char *buffer, int bytes);
	virtual int write(const char *buffer, int bytes);

	void enable(void);
	void acknowledge(void);

} ;

#endif /* IO_SERIAL_H_ */
