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
	ATOSE_circular_buffer<uint8_t, 1024> buffer;

protected:
	void push(uint8_t *byte);

public:
	ATOSE_IO_serial() : ATOSE_IO(), ATOSE_device_driver() {}
	virtual void init(void) { ATOSE_IO::init(); }

	virtual uint32_t read(uint8_t *buffer, uint32_t bytes);
	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes);

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);

} ;

#endif /* IO_SERIAL_H_ */
