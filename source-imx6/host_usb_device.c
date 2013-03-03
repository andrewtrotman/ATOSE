/*
	HOST_USB_DEVICE.C
	-----------------
*/
#include "usb.h"
#include "host_usb.h"
#include "ascii_str.h"
#include "usb_setup_data.h"
#include "host_usb_device.h"
#include "usb_standard_hub_descriptor.h"
#include "usb_standard_device_descriptor.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_standard_configuration_descriptor.h"


/*
	ATOSE_HOST_USB_DEVICE::ATOSE_HOST_USB_DEVICE()
	----------------------------------------------
*/
ATOSE_host_usb_device::ATOSE_host_usb_device(ATOSE_host_usb_device *details)
{
ehci = details->ehci;
vendor_id = details->vendor_id;
product_id = details->product_id;
device_id = details->device_id;
device_class = details->device_class;
device_subclass = details->device_subclass;
device_protocol = details->device_protocol;
max_packet_size = details->max_packet_size;
port_velocity = details->port_velocity;
transaction_translator_address = details->transaction_translator_address;
transaction_translator_port = details->transaction_translator_port;
dead = details->dead;
address = details->address;
parent_address = details->parent_address;
}

/*
	ATOSE_HOST_USB_DEVICE::SEND_SETUP_PACKET()
	------------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::send_setup_packet(uint8_t type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *buffer)
{
ATOSE_usb_setup_data setup_packet;

setup_packet.bmRequestType.all = type;
setup_packet.bRequest = request;
setup_packet.wValue = value;
setup_packet.wIndex = index;
setup_packet.wLength = length;

return ehci->send_setup_packet(this, 0, &setup_packet, buffer, length);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_DESCRIPTOR()
	---------------------------------------
	return 0 on success

*/
uint32_t ATOSE_host_usb_device::get_descriptor(uint8_t target, uint16_t type, void *descriptor, uint32_t descriptor_length)
{
uint32_t error;

/*
	The first time we send a request we don't know how long the descriptor is so we must find out
*/
if ((error = send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, 0x12, descriptor)) != 0)
	return error;

/*
	Make sure the descriptor will fit
*/
if (((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength > descriptor_length)
	return ERROR_OVERFLOW;

/*
	now compute the length and get the descriptor
*/
return send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, ((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength, descriptor);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_CONFIGURATION_DESCRIPTOR()
	-----------------------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::get_configuration_descriptor(ATOSE_usb_standard_configuration_descriptor *descriptor, uint16_t length)
{
uint32_t error;
/*
	The first time we send a request we don't know how long the descriptor is so we must find out
	But, unlike the case with other descriptors, we don't just get the number of bytes in the bLength field, We need to use wTotalLength!
*/
if ((error = send_setup_packet(0x80, ATOSE_usb::REQUEST_GET_DESCRIPTOR, ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION << 8, 0, sizeof(*descriptor), descriptor)) != 0)
	return error;

/*
	Compute the true lengh and go get it
*/
if (descriptor->wTotalLength > length)
	return ERROR_OVERFLOW;
else
	length = descriptor->wTotalLength;

return send_setup_packet(0x80, ATOSE_usb::REQUEST_GET_DESCRIPTOR, ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION << 8, 0, length, descriptor);
}


