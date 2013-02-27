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
uint16_t length = 0x12;
uint32_t error;

/*
	The first time we send a request we don't know how long the descriptor is so we must find out
*/
if ((error = send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, length, descriptor)) != 0)
	return error;

/*
	now compute the length and get the descriptor
*/
length = descriptor_length <= ((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength ? descriptor_length : ((ATOSE_usb_standard_device_descriptor *)descriptor)->bLength;
return send_setup_packet(target, ATOSE_usb::REQUEST_GET_DESCRIPTOR, type << 8, 0, length, descriptor);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_DEVICE_DESCRIPTOR()
	----------------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::get_device_descriptor(ATOSE_usb_standard_device_descriptor *descriptor)
{
return get_descriptor(0x80, ATOSE_usb::DESCRIPTOR_TYPE_DEVICE, descriptor, sizeof(*descriptor));
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
	But, unlike the case with other descriptors, we don't just get the number of bytes in the bLength field,  We need to use wTotalLength!
*/
if ((error = send_setup_packet(0x80, ATOSE_usb::REQUEST_GET_DESCRIPTOR, ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION << 8, 0, sizeof(*descriptor), descriptor)) != 0)
	return error;

/*
	Compute the true lengh and go get it
*/
if (descriptor->wTotalLength < length)
	length = descriptor->wTotalLength;

return send_setup_packet(0x80, ATOSE_usb::REQUEST_GET_DESCRIPTOR, ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION << 8, 0, length, descriptor);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_HUB_DESCRIPTOR()
	-------------------------------------------
	returns 0 on success
*/
uint32_t ATOSE_host_usb_device::get_hub_descriptor(ATOSE_usb_standard_hub_descriptor *descriptor)
{
return get_descriptor(0xA0, ATOSE_usb::DESCRIPTOR_TYPE_HUB, descriptor, sizeof(*descriptor));
}

/*
	ATOSE_HOST_USB_DEVICE::SET_ADDRESS()
	------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::set_address(uint32_t new_address)
{
return send_setup_packet(0, ATOSE_usb::REQUEST_SET_ADDRESS, new_address);
}

/*
	ATOSE_HOST_USB_DEVICE::SET_CONFIGURATION()
	------------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::set_configuration(uint32_t configuration)
{
return send_setup_packet(0, ATOSE_usb::REQUEST_SET_CONFIGURATION, configuration);
}

/*
	ATOSE_HOST_USB_DEVICE::SET_PORT_FEATURE()
	-----------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::set_port_feature(uint32_t port, uint32_t feature)
{
return send_setup_packet(0x23, ATOSE_usb::REQUEST_SET_FEATURE, feature, port);
}

/*
	ATOSE_HOST_USB_DEVICE::CLEAR_PORT_FEATURE()
	-------------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::clear_port_feature(uint32_t port, uint32_t feature)
{
return send_setup_packet(0x23, ATOSE_usb::REQUEST_CLEAR_FEATURE, feature, port);
}

/*
	ATOSE_HOST_USB_DEVICE::GET_PORT_STATUS()
	----------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::get_port_status(uint32_t port, ATOSE_usb_hub_port_status_and_change *answer)
{
return send_setup_packet(0xA3, ATOSE_usb::REQUEST_GET_STATUS, 0, port, 4, answer);
}






/*
	============
	============  DISK STUFF
	============
*/

#include "usb_disk_command_block_wrapper.h"

struct info
{
uint8_t code;
uint8_t evpd;
uint8_t page_code;
uint8_t allocation_length_high;
uint8_t allocation_length_low;
uint8_t control;
} __attribute__ ((packed));

void ATOSE_host_usb_device::get_disk_inquiry(void)
{
uint8_t endpoint_out = 1;
uint8_t endpoint_in = 2;

uint8_t buffer[0xFF];
info *request;
ATOSE_usb_disk_command_block_wrapper command;

command.dCBWSignature = ATOSE_usb_disk_command_block_wrapper::SIGNATURE;
command.dCBWTag = 0x12345678;
command.dCBWDataTransferLength = 0xFF;			// bytes to recv
command.bCBWLUN = 0;
command.bmCBWFlags = ATOSE_usb_disk_command_block_wrapper::FLAG_DIRECTION_IN;
command.bCBWCBLength = sizeof(request);

request = (info *)&command.CBWCB[0];
request->code = 0x12;
request->evpd = 0;
request->page_code = 0;
request->allocation_length_high = 0;
request->allocation_length_low = 0xFF;
request->control = 0;

ehci->send_and_recieve_packet(this, endpoint_out, &command, sizeof(request) + 16, endpoint_in, buffer, sizeof(buffer));

void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
debug_dump_buffer(buffer, 0, sizeof(buffer));
}
