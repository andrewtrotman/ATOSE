/*
	DEVICE_DRIVER.H
	---------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	Generic base class for  device driver.  In the Octopus there are initially only two sources of interrupts,
	the system tick and the USB. Later the PCIe and SATA will get added.
*/
#ifndef DEVICE_DRIVER_H_
#define DEVICE_DRIVER_H_

class ATOSE_registers;

/*
	class ATOSE_DEVICE_DRIVER
	-------------------------
*/
class ATOSE_device_driver
{
public:
	ATOSE_device_driver() {}

	virtual void enable(void) = 0;
	virtual void disable(void) = 0;
	virtual void acknowledge(ATOSE_registers *registers) = 0;

	virtual uint32_t get_interrup_id(void) = 0;
} ;

#endif

