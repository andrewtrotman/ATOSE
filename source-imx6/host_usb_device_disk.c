/*
	HOST_USB_DEVICE_DISK.C
	----------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include "ascii_str.h"
#include "host_usb_device.h"
#include "host_usb_device_disk.h"
#include "host_usb.h"

#include "usb_disk_command_status_wrapper.h"

#include "usb_standard_interface_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"

static uint8_t ATOSE_usb_scsi_inquiry[] =          {0x55, 0x53, 0x42, 0x43, 0x00, 0x41, 0x54, 0x00, 0x24, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x12, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t ATOSE_usb_scsi_read_capacity_10[] = {0x55, 0x53, 0x42, 0x43, 0x00, 0x41, 0x54, 0x00, 0x08, 0x00, 0x00, 0x00, 0x80, 0x00, 0x0A, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_hex(int data);

/*
	ATOSE_HOST_USB_DEVICE_DISK::ATOSE_HOST_USB_DEVICE_DISK()
	--------------------------------------------------------
*/
ATOSE_host_usb_device_disk::ATOSE_host_usb_device_disk(ATOSE_host_usb_device *details) : ATOSE_host_usb_device(details)
{
ATOSE_usb_standard_configuration_descriptor *configuration;
ATOSE_usb_standard_interface_descriptor *interface;
ATOSE_usb_standard_endpoint_descriptor *endpoint;
uint8_t buffer[64];			// I doubt it'll be longer than this

/*
	Find the endpoints (which do differ from disk to disk (coz I checked)
*/
configuration = (ATOSE_usb_standard_configuration_descriptor *)buffer;
if (get_configuration_descriptor(configuration, sizeof(buffer)) != 0)
	return;

interface = (ATOSE_usb_standard_interface_descriptor *)(configuration + 1);
endpoint = (ATOSE_usb_standard_endpoint_descriptor *)(interface + 1);
if (endpoint->bEndpointAddress & ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN)
	endpoint_in = endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN;
else
	endpoint_out = endpoint->bEndpointAddress;

endpoint++;

if (endpoint->bEndpointAddress & ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN)
	endpoint_in = endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN;
else
	endpoint_out = endpoint->bEndpointAddress;

dead = false;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::GET_DISK_INQUIRY()
	----------------------------------------------
*/
uint32_t ATOSE_host_usb_device_disk::get_disk_inquiry(void)
{
uint32_t error;
uint8_t buffer[512];

if ((error = get_max_lun(&logical_units)) == 0)
	{
	/*
		The device return the highest LUN number, we want the number of LUNs
	*/
	logical_units++;

	memset(buffer, 0, sizeof(buffer));

	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_inquiry, sizeof(ATOSE_usb_scsi_inquiry), endpoint_in, buffer, 0x24);
	buffer[32] = '\0';
	debug_print_string((char *)(buffer + 8));
	//debug_dump_buffer(buffer, 0, 96);
	//debug_print_string("\r\n");

	//debug_print_string("REC:\r\n");
	//memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);
	ATOSE_usb_disk_command_status_wrapper *ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);

	//debug_dump_buffer(buffer, 0, 13);
	//debug_print_string("\r\n");

//	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_capacity_10, sizeof(ATOSE_usb_scsi_read_capacity_10), endpoint_in, buffer, 0x08);
//	ehci->recieve_packet(this, endpoint_in, buffer, 13);

//	debug_print_string("(");
//	debug_dump_buffer(buffer, 0, 8);
//	debug_print_string(")");

	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_inquiry, sizeof(ATOSE_usb_scsi_inquiry), endpoint_in, buffer, 0x24);
	buffer[32] = '\0';
	debug_print_string((char *)(buffer + 8));
	}
return error;
}
