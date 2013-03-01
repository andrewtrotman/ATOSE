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
	ATOSE_HOST_USB_DEVICE::SET_INTERFACE()
	--------------------------------------
	return 0 on success
*/
uint32_t ATOSE_host_usb_device::set_interface(uint32_t interface)
{
return send_setup_packet(0, ATOSE_usb::REQUEST_SET_INTERFACE, interface);
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

uint32_t ATOSE_host_usb_device::clear_feature_halt_disk(uint32_t endpoint)
{
return send_setup_packet(0x02, ATOSE_usb::REQUEST_CLEAR_FEATURE, endpoint);
}


uint32_t ATOSE_host_usb_device::reset_disk(void)
{
return send_setup_packet(0x21, 0xFF);
}

uint32_t ATOSE_host_usb_device::get_max_lun(uint32_t *luns)
{
uint32_t answer;
uint8_t byte;

answer = send_setup_packet(0xA1, 0xFE, 0, 0, 1, &byte);
*luns = byte;

return answer;
}

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

void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");

uint32_t  ATOSE_host_usb_device::get_disk_inquiry(void)
{
uint8_t endpoint_out = 1;
uint8_t endpoint_in = 2;
uint32_t luns;

uint8_t buffer[512];
info *request;
ATOSE_usb_disk_command_block_wrapper command;

/*
	Set the interface to 0
*/
//debug_print_string("DISK Set INTERFACE");
//set_interface(1);

//debug_print_string("\r\nSet Configuration\r\n");
//set_configuration(1);

//debug_print_string("\r\nSet interface\r\n");
//set_interface(0);

//debug_print_string("RESET DISK\r\n");
//reset_disk();

luns = 1024;
debug_print_string("GET MAX LUNS\r\n");
get_max_lun(&luns);
debug_print_this("Luns:", luns);

memset(&command, 0, sizeof(command));
memset(buffer, 0, sizeof(buffer));

/*
55 53 42 43
01 00 00 00
00 00 00 00
00
00
06
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*/

command.dCBWSignature = ATOSE_usb_disk_command_block_wrapper::SIGNATURE;
command.dCBWTag = 1;
command.dCBWDataTransferLength = 0x24;			// bytes in
command.bmCBWFlags = 0x80;
command.bCBWLUN = 0;
command.bCBWCBLength = 0x06;

request = (info *)&command.CBWCB[0];
request->code = 0x12;
request->evpd = 0;
request->page_code = 0;
request->allocation_length_high = 0;
request->allocation_length_low = 0x24;
request->control = 0;

/*
55 53 42 43
00 DE 2E 82
24 00 00 00
80
00
06
12 00 00 00 24 00 00 00 00 00 00 00 00 00 00 00
*/

#ifdef NEVER
debug_print_string("SEND:\r\n");
debug_dump_buffer((unsigned char *)&command, 0, 31);

memset(buffer, 0, sizeof(buffer));
ehci->send_packet(this, endpoint_out, &command, 31);

debug_print_string("REC:\r\n");
ehci->recieve_packet(this, endpoint_in, buffer, 0x24);
#else
debug_print_string("SEND RECV:\r\n");
debug_dump_buffer((unsigned char *)&command, 0, 31);

ehci->send_and_recieve_packet(this, endpoint_out, &command, 31, endpoint_in, buffer, 0x24);

#endif

debug_dump_buffer(buffer, 0, sizeof(buffer));
debug_print_string("\r\n");

return 0;
}
