/*
	HOST_USB_DEVICE_HUB.H
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef HOST_USB_DEVICE_HUB_H_
#define HOST_USB_DEVICE_HUB_H_

#include "host_usb_device.h"
#include "host_usb_device_generic.h"
#include "usb_standard_hub_descriptor.h"

class ATOSE_usb_hub_port_status_and_change;
class ATOSE_usb_standard_configuration_descriptor;

/*
	class ATOSE_HOST_USB_DEVICE_HUB
	-------------------------------
*/
class ATOSE_host_usb_device_hub : public ATOSE_host_usb_device
{
public:
	uint8_t hub_ports;

protected:
	void hub_get_best_configuration(ATOSE_usb_standard_configuration_descriptor *configuration_descriptor, uint32_t *best_interface, uint32_t *best_alternate, uint32_t *best_endpoint);

public:
	ATOSE_host_usb_device_hub(ATOSE_host_usb_device *details);

	/*
		All of these methods return 0 on success
	*/
	uint32_t get_hub_descriptor(ATOSE_usb_standard_hub_descriptor *descriptor)            { return get_descriptor(0xA0, ATOSE_usb::DESCRIPTOR_TYPE_HUB, descriptor, sizeof(*descriptor)); }
	uint32_t set_port_feature(uint32_t port, uint32_t feature)                            { return send_setup_packet(0x23, ATOSE_usb::REQUEST_SET_FEATURE, feature, port); }
	uint32_t clear_port_feature(uint32_t port, uint32_t feature)                          { return send_setup_packet(0x23, ATOSE_usb::REQUEST_CLEAR_FEATURE, feature, port); }
	uint32_t get_port_status(uint32_t port, ATOSE_usb_hub_port_status_and_change *answer) { return send_setup_packet(0xA3, ATOSE_usb::REQUEST_GET_STATUS, 0, port, 4, answer); }
} ;

static_assert(sizeof(ATOSE_host_usb_device_generic) > sizeof(ATOSE_host_usb_device_hub), "Increase the padding in ATOSE_host_usb_device_generic so that it is at lease the size of ATOSE_host_usb_device_hub");

#endif
