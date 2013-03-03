/*
	HOST_USB_DEVICE.H
	-----------------
*/
#ifndef HOST_USB_DEVICE_H_
#define HOST_USB_DEVICE_H_

#include <stdlib.h>
#include <stdint.h>

/*
	We need to declare this before the includes because of a mutual inclusion problem
*/
class ATOSE_host_usb;

#include "usb.h"
#include "usb_standard_hub_descriptor.h"
#include "usb_standard_device_descriptor.h"
#include "usb_standard_configuration_descriptor.h"

/*
	class ATOSE_HOST_USB_DEVICE
	---------------------------
*/
class ATOSE_host_usb_device
{
public:
	/*
		Possible error codes
	*/
	enum { ERROR_NONE = 0, ERROR_OVERFLOW };

public:
	/*
		These velocity values are chosen because they are the values
		used in the Endpoint Characteristics (see Page 5218 of "i.MX
		6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
		These values should probably no be changed.
	*/
	static const uint8_t VELOCITY_LOW =	 1;	// 1.5 Mb/s
	static const uint8_t VELOCITY_FULL = 0;	//  12 Mb/s
	static const uint8_t VELOCITY_HIGH = 2;	// 480 Mb/s

public:
	ATOSE_host_usb *ehci;

	/*
		Information about the device itself.
	*/
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t device_id;

	/*
		Information about what it is
	*/
	uint8_t device_class;
	uint8_t device_subclass;
	uint8_t device_protocol;
	/*
		Stuff about how to talk to it
	*/
	uint16_t max_packet_size;
	uint8_t port_velocity;
	uint8_t transaction_translator_address;			// this is defined as a 7-bit number in ATOSE_usb_ehci_queue_head_endpoint_capabilities
	uint8_t transaction_translator_port;				// this is defined as a 7-bit number in ATOSE_usb_ehci_queue_head_endpoint_capabilities

	/*
		Stuff about the object itself
	*/
	uint8_t dead;												// do we want to manage this object?  if not the dead == true;
	uint8_t address;											// the object's address on the USB bus
	uint8_t parent_address;									// the address of the object's parent on the USB bus (necessary for disconnect of a hub containing children).

protected:
	uint32_t send_setup_packet(uint8_t type = 0, uint8_t request = 0, uint16_t value = 0, uint16_t index = 0, uint16_t length = 0, void *buffer = NULL);
	uint32_t get_descriptor(uint8_t target, uint16_t type, void *descriptor, uint32_t descriptor_length);

public:
	ATOSE_host_usb_device() {}
	ATOSE_host_usb_device(ATOSE_host_usb_device *details);

	/*
		Device methods.  These all return 0 on success
	*/
	uint32_t get_configuration_descriptor(ATOSE_usb_standard_configuration_descriptor *descriptor, uint16_t length = 0);
	uint32_t get_device_descriptor(ATOSE_usb_standard_device_descriptor *descriptor)  { return get_descriptor(0x80, ATOSE_usb::DESCRIPTOR_TYPE_DEVICE, descriptor, sizeof(*descriptor)); }
	uint32_t set_address(uint32_t new_address)                                        { return send_setup_packet(0, ATOSE_usb::REQUEST_SET_ADDRESS, new_address); }
	uint32_t set_interface(uint32_t interface)                                        { return send_setup_packet(0, ATOSE_usb::REQUEST_SET_INTERFACE, interface); }
	uint32_t set_configuration(uint32_t configuration)                                { return send_setup_packet(0, ATOSE_usb::REQUEST_SET_CONFIGURATION, configuration); }
};

#endif
