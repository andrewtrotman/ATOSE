/*
	IO_SERIAL.H
	-----------
*/
#ifndef IO_SERIAL_H_
#define IO_SERIAL_H_

#include "io.h"
#include "device_driver.h"
#include "circular_buffer.h"

/*
	class ATOSE_IO_SERIAL
	---------------------
*/
class ATOSE_IO_serial : public ATOSE_IO, public ATOSE_device_driver
{
private:
	ATOSE_circular_buffer<unsigned char, 1024> buffer;

protected:
	void push(unsigned char *byte);

public:
	ATOSE_IO_serial() : ATOSE_IO(), ATOSE_device_driver() {}
	virtual void init(void) { ATOSE_IO::init(); }

	virtual int read(char *buffer, int bytes);
	virtual int write(const char *buffer, int bytes);

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);

} ;

#endif /* IO_SERIAL_H_ */
