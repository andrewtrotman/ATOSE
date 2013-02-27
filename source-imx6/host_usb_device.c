/*
	HOST_USB_DEVICE.C
	-----------------
*/
#include "usb.h"
#include "host_usb.h"
#include "usb_setup_data.h"
#include "host_usb_device.h"
#include "usb_standard_hub_descriptor.h"
#include "usb_standard_device_descriptor.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_standard_configuration_descriptor.h"

/*
	ATOSE_HOST_USB_DEVICE::SEND_SETUP_PACKET()
	------------------------------------------
*/
void *ATOSE_host_usb_device::send_setup_packet(uint8_t type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *buffer)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = type;
setup_packet.bRequest = request;
setup_packet.wValue = value;
setup_packet.wIndex = index;
setup_packet.wLength = length;

// worry about timeout here.
// return NULL on fail else return buffer (which might be NULL so fix this)
return ehci->send_setup_packet(this, 0, &setup_packet, buffer, length);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_DESCRIPTOR()
	---------------------------------------
	return NULL on error
*/
void *ATOSE_host_usb_device::get_descriptor(uint8_t target, uint16_t type, void *descriptor, uint32_t descriptor_length)
{
uint16_t length = 0x12;

/*
	The first time we send a request we don't know how long the descriptor is so we must find out
*/
if (send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, length, descriptor) != NULL)
	{
	length = descriptor_length <= ((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength ? descriptor_length : ((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength;
	return send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, length, descriptor);
	}

return NULL;
}

/*
	ATOSE_HOST_USB_DEVICE::GET_DEVICE_DESCRIPTOR()
	----------------------------------------------
	return NULL on error
*/
ATOSE_usb_standard_device_descriptor *ATOSE_host_usb_device::get_device_descriptor(ATOSE_usb_standard_device_descriptor *descriptor)
{
return (ATOSE_usb_standard_device_descriptor *)get_descriptor(0x80, ATOSE_usb::DESCRIPTOR_TYPE_DEVICE, descriptor, sizeof(*descriptor));
}

/*
	ATOSE_HOST_USB_DEVICE::GET_CONFIGURATION_DESCRIPTOR()
	-----------------------------------------------------
	return NULL on error
*/
ATOSE_usb_standard_configuration_descriptor *ATOSE_host_usb_device::get_configuration_descriptor(ATOSE_usb_standard_configuration_descriptor *descriptor)
{
return (ATOSE_usb_standard_configuration_descriptor *)get_descriptor(0x80, ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION, descriptor, sizeof(*descriptor));
}

/*
	ATOSE_HOST_USB_DEVICE::GET_HUB_DESCRIPTOR()
	-------------------------------------------
	return NULL on error
*/
ATOSE_usb_standard_hub_descriptor *ATOSE_host_usb_device::get_hub_descriptor(ATOSE_usb_standard_hub_descriptor *descriptor)
{
if (device_class != ATOSE_usb_standard_interface_descriptor::CLASS_HUB)
	return NULL;
return (ATOSE_usb_standard_hub_descriptor *)get_descriptor(0xA0, ATOSE_usb::DESCRIPTOR_TYPE_HUB, descriptor, sizeof(*descriptor));
}

/*
	ATOSE_HOST_USB_DEVICE::SET_ADDRESS()
	------------------------------------
	return -1 on error else the address that was assumed
*/
int32_t ATOSE_host_usb_device::set_address(uint32_t new_address)
{
return send_setup_packet(0, ATOSE_usb::REQUEST_SET_ADDRESS, new_address) == NULL ? -1 : new_address;
}

/*
	ATOSE_HOST_USB_DEVICE::SET_CONFIGURATION()
	------------------------------------------
	return -1 on error else the configuration that was assumed
*/
int32_t ATOSE_host_usb_device::set_configuration(uint32_t configuration)
{
return send_setup_packet(0, ATOSE_usb::REQUEST_SET_CONFIGURATION, configuration) == NULL ? -1 : configuration;
}

/*
	ATOSE_HOST_USB_DEVICE::SET_PORT_FEATURE()
	-----------------------------------------
	return -1 on error else the port number
*/
int32_t ATOSE_host_usb_device::set_port_feature(uint32_t port, uint32_t feature)
{
if (device_class != ATOSE_usb_standard_interface_descriptor::CLASS_HUB)
	return -1;

if (send_setup_packet(0x23, ATOSE_usb::REQUEST_SET_FEATURE, feature, port) == NULL)
	return -1;

return port;
}

/*
	ATOSE_HOST_USB_DEVICE::CLEAR_PORT_FEATURE()
	-------------------------------------------
	return -1 on error else the port number
*/
int32_t ATOSE_host_usb_device::clear_port_feature(uint32_t port, uint32_t feature)
{
if (device_class != ATOSE_usb_standard_interface_descriptor::CLASS_HUB)
	return -1;

if (send_setup_packet(0x23, ATOSE_usb::REQUEST_CLEAR_FEATURE, feature, port) == NULL)
	return -1;

return port;
}

/*
	ATOSE_HOST_USB_DEVICE::GET_PORT_STATUS()
	----------------------------------------
	return -1 on error else the port number

*/
int32_t ATOSE_host_usb_device::get_port_status(uint32_t port, uint32_t *answer)
{
if (device_class != ATOSE_usb_standard_interface_descriptor::CLASS_HUB)
	return -1;

if (send_setup_packet(0xA3, ATOSE_usb::REQUEST_GET_STATUS, 0, port, 4, answer) == NULL)
	return -1;

return port;
}

