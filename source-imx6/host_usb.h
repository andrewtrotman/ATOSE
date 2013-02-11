/*
	HOST_USB.H
	----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_H_
#define HOST_USB_H_

#include "device_driver.h"

/*
	class ATOSE_HOST_USB
	--------------------
*/
class ATOSE_host_usb : public ATOSE_device_driver
{
public:
	ATOSE_host_usb();

	virtual void enable(void);
	virtual void disable(void) {}
	virtual void acknowledge(void);

	virtual uint32_t get_interrup_id(void);
} ;

#endif /* HOST_USB_H_ */
