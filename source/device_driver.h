/*
	DEVICE_DRIVER.H
	---------------
*/
#ifndef DEVICE_DRIVER_H_
#define DEVICE_DRIVER_H_

/*
	class ATOSE_DEVICE_DRIVER
	-------------------------
*/
class ATOSE_device_driver
{
public:
	ATOSE_device_driver() {}
	virtual void init(void) {}

	virtual void enable(void);
	virtual void disable(void);
	virtual void acknowledge(void);
} ;

#endif /* DEVICE_DRIVER_H_ */
