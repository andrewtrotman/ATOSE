/*
	USB_IMX6Q.H
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_IMX6Q_H_
#define USB_IMX6Q_H_

#include <stdint.h>
#include "usb.h"
#include "usb_imx6q_endpoint_queuehead.h"
#include "usb_imx6q_endpoint_transfer_descriptor.h"

/*
	We need to tell the i.MX6 SDK that we're using an i.MX6Q
*/
#define CHIP_MX6DQ 1

/*
	class ATOSE_USB_IMX6Q
	---------------------
*/
class ATOSE_usb_imx6q : public ATOSE_usb
{
public:
	/*
		a transfer can do at most 4096 bytes of memory per buffer (due to the way counting is done).
		See page 7 of "Simplified Device Data Structures for the High-End ColdFire Family USB Modules"
	*/
	static const uint32_t QUEUE_BUFFER_SIZE = 4096;

	/*
		The i.MX53 and i.MX6Q have 8 endpoints 0..7
			see page 5449 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
			see page 4956 of "i.MX53 Multimedia Applications Processor Reference Manual Rev. 2.1, 06/2012"
	*/
	static const uint32_t MAX_ENDPOINTS = 8;

	/*
		There are two queue heads for each endpoint, even numbers are OUT going from host, odd numbers 
		are IN coming to host. "The Endpoint Queue Head List must be aligned to a 2k boundary"  see page
		5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	*/
	ATOSE_usb_imx6q_endpoint_queuehead port_queuehead[MAX_ENDPOINTS * 2] __attribute__ ((aligned (2048)));

	/*
		Each endpoint has one endpoint transfer descriptor for each direction.  Each must be aligned on 32-byte
		boundaries because the last 5 bits must be zero.  Note that the last bit of a pointer is a termination bit (T)
		see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	*/
	ATOSE_usb_imx6q_endpoint_transfer_descriptor port_transfer_descriptor[MAX_ENDPOINTS * 2] __attribute__ ((aligned(32)));

	/*
		Each transfer descriptor has a pointer to 5 separate transfer buffers. We're only going to use one of these but we'll
		allocate the space to keep all the pointers in case we need more

		"Buffer Pointer. Indicates the physical memory address for the data buffer to be used by the
		dTD. The device controller actually uses the first part of the address (bits 31-12) as a
		pointer to a 4 KB page, and the lower part of the address (bits 11-0) as an index into the
		page. The host controller will increment the index internally, but will not increment the page
		address. This is what determines the 4 KB transfer size limitation used for this application
		note." see page 7 of "Simplified Device Data Structures for the High-End ColdFire Family USB Modules"
	*/
	uint8_t *global_transfer_buffer[MAX_ENDPOINTS * 2][ATOSE_usb_imx6q_endpoint_transfer_descriptor::BUFFER_POINTERS];

private:
	void portchange_interrupt(void);
	void reset_interrupt(void);
	void usb_interrupt(void);
	void configure_queuehead(long which);

protected:
	virtual void send_to_host(uint32_t endpoint, const uint8_t *buffer, uint32_t length);
	virtual void recieve_from_host(uint32_t endpoint);
	virtual void signal_an_error(uint32_t endpoint);
	virtual void enable_endpoint_zero(void);
	virtual void enable_endpoint(long endpoint, long mode);
	virtual void set_address(long address);

public:
	ATOSE_usb_imx6q();

	virtual void enable(void);
	virtual void disable(void) {}
	virtual void acknowledge(void);
} ;

#endif
