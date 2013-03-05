/*
	HOST_USB_DEVICE_DISK.C
	----------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Apparently, SCSI requires the following... but other than from some chip manufacturer's datasheet I cannot find reference to this...
		Inquiry
		Request Sense
		Test Unit Ready
		Read(10)
		Read Capacity(10)
		Write(10)
	the number in brackets e.g. "(10)" is the length of the command.

*/

#include "fat.h"

#include "atose.h"
#include "ascii_str.h"
#include "host_usb_device.h"
#include "host_usb_device_disk.h"
#include "host_usb.h"

#include "usb_disk_command_status_wrapper.h"
#include "usb_standard_interface_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"

#include "scsi_read_capacity_10_parameter_data.h"
#include "scsi_read_capacity_16_parameter_data.h"

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_test_unit_ready[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x00, 0x00, 0x00, // dCBWDataTransferLength
0x00,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x06,                   // bCBWCBLength
0x00,                   // SCSI Command
0x00,                   // SCSI LUN
0x00,                   // SCSI Reserved
0x00,                   // SCSI Reserved
0x00,                   // SCSI Reserved
0x00                    // SCSI Control
};

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_read_capacity_10[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x08, 0x00, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x0A,                   // bCBWCBLength
0x25,                   // SCSI Command
0x00,                   // SCSI RelAddr and Lun
0x00, 0x00, 0x00, 0x00, // SCSI LBA
0x00,                   // SCSI Reserved
0x00,                   // SCSI Reserved
0x00,                   // SCSI PMI
0x00                    // SCSI Control
};

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_read_capacity_16[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x20, 0x00, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x10,                   // bCBWCBLength
0x9E,                   // SCSI Command
0x10,                   // SCSI Service Action
0x00, 0x00, 0x00, 0x00, // SCSI LBA Address
0x00, 0x00, 0x00, 0x00, // SCSI LBA Address
0x00, 0x00, 0x00, 0x20, // SCSI Allocation Length	 (send me 32-bytes)
0x00,                   // SCSI PMI
0x00                    // SCSI Control
};

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_read_10[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x02, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x0A,                   // bCBWCBLength
0x28,                   // SCSI Command
0x00,                   // SCSI flags
0x00,                   // SCSI MSB Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI LSB Address
0x00,                   // SCSI Group number
0x00,                   // SCSI MSB Transfer Length
0x01,                   // SCSI LSB Transfer Length
0x00                    // SCSI Control
};

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_read_16[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x02, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x10,                   // bCBWCBLength
0x88,                   // SCSI Command
0x00,                   // SCSI flags
0x00,                   // SCSI MSB Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI LSB Address
0x00,                   // SCSI MSB Transfer Length
0x00,                   // SCSI     Transfer Length
0x00,                   // SCSI     Transfer Length
0x01,                   // SCSI LSB Transfer Length
0x00,                   // SCSI Group number
0x00                    // SCSI Control
};








static uint8_t ATOSE_usb_scsi_request_sense[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	18, 0x00, 0x00, 0x00, 	// dCBWDataTransferLength
	0x80, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	0x06, 						// bCBWCBLength

	0x03, 						// SCSI Command
	0x00, 						// SCSI Reserved
	0x00, 0x00, 				// SCSI Reserved
	18, 						// SCSI Allocation Length
	0x00						// SCSI Control
	};


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
uint32_t time_to_spin_up, error;

/*
	Find the endpoints (which do differ from disk to disk (coz I checked)
*/
configuration = (ATOSE_usb_standard_configuration_descriptor *)buffer;
if (get_configuration_descriptor(configuration, sizeof(buffer)) != 0)
	return;

/*
	The configuration descriptor should have one interface descriptor
	The interface descriptor should have 2 endpoiunts descriptors, one for in and the other for out
*/
interface = (ATOSE_usb_standard_interface_descriptor *)(configuration + 1);
endpoint = (ATOSE_usb_standard_endpoint_descriptor *)(interface + 1);

/*
	Find out if we're the in or out BULK endpoint, record this, and get the max packet size
*/
if (endpoint->bEndpointAddress & ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN)
	endpoint_in = endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN;
else
	endpoint_out = endpoint->bEndpointAddress;

max_packet_size[endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN] = endpoint->wMaxPacketSize;

/*
	move on to the second endpoint descriptor
*/
endpoint++;

/*
	Find out if we're the in or out BULK endpoint, record this, and get the max packet size
*/
if (endpoint->bEndpointAddress & ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN)
	endpoint_in = endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN;
else
	endpoint_out = endpoint->bEndpointAddress;

max_packet_size[endpoint->bEndpointAddress & ~ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN] = endpoint->wMaxPacketSize;

/*
	Now we bring the disk up
*/
if ((error = get_max_lun(&logical_units)) != 0)
	return;

/*
	The device return the highest LUN number, we want the number of LUNs
*/
logical_units++;

/*
	Wait for the device to spin-up.  This can take considerable time even on a USB Flash Disk
*/
time_to_spin_up = 0;
while (scsi_test_unit_ready() != 0)
	{
	ATOSE_atose::get_ATOSE()->cpu.delay_us(20000);		// wait 20ms and try again
	if ((time_to_spin_up += 20000) > 1000000)
		return;														// If it took more than 1 second to spin up we're a dead disk.
	}

/*
	find out how large the disk is.  First you must check the 32-bit version and if that
	return 0xFFFFFFFF then you must check the 64-bit version.
*/
block_count = block_size = 0;
if (scsi_read_capacity_10(&block_count, &block_size) != 0)
	return;
if (block_count == 0xFFFFFFFF)
	{
	/*
		I can't test this at the moment because I don't have a disk larget than 2TB to test it on.
	*/
	if (scsi_read_capacity_16(&block_count, &block_size) != 0)
		return;
	}

/*
	We're up and running as and endpoint (even if we're not running as a disk yet).
*/
dead = false;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::PERFORM_TRANSACTION()
	-------------------------------------------------
	return 0 on success anything else is an error
*/
uint32_t ATOSE_host_usb_device_disk::perform_transaction(uint8_t *command, void *buffer, uint32_t buffer_length)
{
uint32_t error;
ATOSE_usb_disk_command_status_wrapper reply;

memset(&reply, 0, sizeof(reply));
if (buffer == NULL)
	{
	if ((error = ehci->send_and_recieve_packet(this, endpoint_out, command, 31, endpoint_in, &reply, sizeof(reply))) != 0)
		return error;
	}
else
	{
	if ((error = ehci->send_and_recieve_packet(this, endpoint_out, command, 31, endpoint_in, buffer, buffer_length)) != 0)
		return error;

	if ((error = ehci->recieve_packet(this, endpoint_in, &reply, sizeof(reply))) != 0)
		return error;
	}

//#ifdef NEVER
	debug_print_this("SIG:", reply.dCSWSignature);
	debug_print_this("TAG:", reply.dCSWTag);
	debug_print_this("RES:", reply.dCSWDataResidue);
	debug_print_this("STS:", reply.bCSWStatus);
//#endif

/*
	The USB transaction was successfully transmitted, but we don't know if the the SCSI command was successful or not
*/
if (reply.bCSWStatus != 0 || reply.dCSWSignature != ATOSE_usb_disk_command_status_wrapper::SIGNATURE)
	return 1;

return 0;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::SCSI_READ_CAPACITY_10()
	---------------------------------------------------
	This is the 32-bit version
*/
uint32_t ATOSE_host_usb_device_disk::scsi_read_capacity_10(uint64_t *count, uint64_t *size)
{
ATOSE_scsi_read_capacity_10_parameter_data specification;
uint32_t error;

if ((error = perform_transaction(ATOSE_usb_scsi_read_capacity_10, &specification, sizeof(specification))) != 0)
	return error;

*count = specification.returned_logical_block_address;
*size = specification.block_length_in_bytes;

return 0;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::SCSI_READ_CAPACITY_16()
	---------------------------------------------------
	this is the 64-bit version
*/
uint32_t ATOSE_host_usb_device_disk::scsi_read_capacity_16(uint64_t *count, uint64_t *size)
{
ATOSE_scsi_read_capacity_16_parameter_data specification;
uint32_t error;

if ((error = perform_transaction(ATOSE_usb_scsi_read_capacity_16, &specification, sizeof(specification))) != 0)
	return error;

*count = specification.returned_logical_block_address;
*size = specification.block_length_in_bytes;

return 0;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::SCSI_READ()
	---------------------------------------
*/
uint32_t ATOSE_host_usb_device_disk::scsi_read(uint8_t *buffer, uint32_t buffer_length, uint64_t block, uint32_t blocks_to_read)
{
ATOSE_msb_uint64_t *address_64;
ATOSE_msb_uint32_t *address_32, *number_32;
ATOSE_msb_uint16_t *number_16;
uint8_t command[31];

if (block < 0xFFFFFFFF && blocks_to_read < 0xFFFF)
	{
	/*
		We can do a 32-bit transfer
	*/
	memcpy(command, ATOSE_usb_scsi_read_10, sizeof(command));

	address_32 = (ATOSE_msb_uint32_t *)(command + 17);
	*address_32 = (uint32_t)block;
	number_16 = (ATOSE_msb_uint16_t *)(command + 22);
	*number_16 = (uint16_t)blocks_to_read;
	}
else
	{
	/*
		We do a 64-bit transfer.  This code has not been tested because I don't have a large enough disk
	*/
	memcpy(command, ATOSE_usb_scsi_read_16, sizeof(command));
	address_64 = (ATOSE_msb_uint64_t *)(command + 17);
	*address_64 = block;
	number_32 = (ATOSE_msb_uint32_t *)(command + 25);
	*number_32 = (uint32_t)blocks_to_read;
	}

return perform_transaction(command, buffer, buffer_length);
}


/*
	ATOSE_HOST_USB_DEVICE_DISK::GET_DISK_INQUIRY()
	----------------------------------------------
*/
uint32_t ATOSE_host_usb_device_disk::get_disk_inquiry(void)
{
uint32_t block;
uint32_t error;
uint8_t buffer[512], *partition_table;

if ((error = scsi_read(buffer, 512, block, 1)) != 0)
	return 0;
#ifdef NEVER
	debug_print_this("\r\n\r\nBLOCK:", block);
	debug_dump_buffer(buffer, 0, 512);
#endif

partition_table = buffer + 0x1BE;

//#ifdef NEVER
	debug_print_string("Partition 1\r\n");
	debug_print_this("status         :", partition_table[0]);
	debug_print_this("start address H:", partition_table[1]);
	debug_print_this("start address S:", partition_table[2] & 0x3F);
	debug_print_this("start address T:", ((partition_table[2] >> 5) << 10) | partition_table[3]);
	debug_print_this("type           :", partition_table[4]);
	debug_print_this("end address H  :", partition_table[5]);
	debug_print_this("end address S  :", partition_table[6] & 0x3F);
	debug_print_this("end address T  :", ((partition_table[6] >> 5) << 10) | partition_table[7]);
	debug_print_this("First LBA      :", (uint32_t)(*(ATOSE_lsb_uint32_t *)(partition_table + 0x08)));
	debug_print_this("len in sectors :", (uint32_t)(*(ATOSE_lsb_uint32_t  *)(partition_table + 0x0C)));
//#endif

ATOSE_fat fat(this, (uint32_t)(*(ATOSE_lsb_uint32_t *)(partition_table + 0x08)));
fat.dir();

return error;
}
