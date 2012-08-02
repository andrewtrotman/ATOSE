/*
	PIC.H
	-----
	Programmable Interrupt controller
*/
#ifndef PIC_H_
#define PIC_H_

#include <stdint.h>
#include "device_driver.h"
#include "registers.h"

/*
	class ATOSE_PIC
	---------------
*/
class ATOSE_pic : public ATOSE_device_driver
{
protected:
	/*
		Sentinal constants
	*/
	static const uint32_t NO_SECONDARY = 0xFFFFFFFF;

public:
	ATOSE_pic() : ATOSE_device_driver() {}
	virtual void init(void) = 0;

	virtual void enable(ATOSE_device_driver *driver, uint32_t primary, uint32_t secondary = NO_SECONDARY) = 0;
	virtual void acknowledge(void) = 0;
} ;



#endif /* PIC_H_ */
