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

#include "usb_disk_command_block_wrapper.h"

#include "file_control_block.h"

/*
	========================
	========================
	========================
*/
void debug_dump_buffer(unsigned char *buffer, uint32_t address, uint64_t bytes);
void debug_print_string(const char *string);
void debug_print_this(const char *start, uint32_t hex, const char *end = "");
void debug_print_hex(int data);
/*
	========================
	========================
	========================
*/

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

static uint8_t ATOSE_usb_scsi_request_sense[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x12, 0x00, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x06,                   // bCBWCBLength
0x03,                   // SCSI Command
0x00,                   // SCSI Reserved
0x00,                   // SCSI Reserved
0x00,                   // SCSI Reserved
0x12,                   // SCSI Transfer Length
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

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_write_10[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x02, 0x00, 0x00, // dCBWDataTransferLength
0x00,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x0A,                   // bCBWCBLength
0x2A,                   // SCSI Command
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

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_write_16[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x02, 0x00, 0x00, // dCBWDataTransferLength
0x00,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x10,                   // bCBWCBLength
0x8A,                   // SCSI Command
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

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_synchoronise_cache_10[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x00, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x0A,                   // bCBWCBLength
0x35,                   // SCSI Command
0x00,                   // SCSI flags
0x00,                   // SCSI MSB Address
0x00,                   // SCSI     Address
0x00,                   // SCSI     Address
0x00,                   // SCSI LSB Address
0x00,                   // SCSI Group number
0x00,                   // SCSI MSB Transfer Length
0x00,                   // SCSI LSB Transfer Length
0x00                    // SCSI Control
};

uint8_t ATOSE_host_usb_device_disk::ATOSE_usb_scsi_synchoronise_cache_16[31] =
{
0x55, 0x53, 0x42, 0x43, // dCBWSignature
0x00, 0x41, 0x54, 0x00, // dCBWTag
0x00, 0x02, 0x00, 0x00, // dCBWDataTransferLength
0x80,                   // bmCBWFlags
0x00,                   // bCBWLUN
0x10,                   // bCBWCBLength
0x91,                   // SCSI Command
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
	/*
		This is when we don't do a transfer in either direction (e.g. TEST UNIT READY)
	*/
	if ((error = ehci->send_and_recieve_packet(this, endpoint_out, command, 31, endpoint_in, &reply, sizeof(reply))) != 0)
		return error;
	}
else if ((command[12] & ATOSE_usb_disk_command_block_wrapper::FLAG_DIRECTION_IN) != 0)			// check the flags to see which direction we are going in (data to / from the device)
	{
	/*
		This is when we perform a transfer and the device sends us data back (e.g. READ)
	*/
	if ((error = ehci->send_and_recieve_packet(this, endpoint_out, command, 31, endpoint_in, buffer, buffer_length)) != 0)
		return error;

	if ((error = ehci->recieve_packet(this, endpoint_in, &reply, sizeof(reply))) != 0)
		return error;
	}
else
	{
	/*
		This is when we perform a transfer and must send data to the device (e.g. WRITE)
	*/
	if ((error = ehci->send_and_send_packet(this, endpoint_out, command, 31, buffer, buffer_length)) != 0)
		return error;

	if ((error = ehci->recieve_packet(this, endpoint_in, &reply, sizeof(reply))) != 0)
		return error;
	}

#ifdef NEVER
	debug_print_this("SIG:", reply.dCSWSignature);
	debug_print_this("TAG:", reply.dCSWTag);
	debug_print_this("RES:", reply.dCSWDataResidue);
	debug_print_this("STS:", reply.bCSWStatus);
#endif

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
ATOSE_lsb_uint32_t *bytes_to_transfer;
uint8_t command[31];

/*
	Set up the USB transfer
*/
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

/*
	Tell the USB how many bytes to send us
*/
bytes_to_transfer = (ATOSE_lsb_uint32_t *)(command + 8);
*bytes_to_transfer = block_size * blocks_to_read;

return perform_transaction(command, buffer, buffer_length);
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::SCSI_WRITE()
	----------------------------------------
*/
uint32_t ATOSE_host_usb_device_disk::scsi_write(uint8_t *buffer, uint32_t buffer_length, uint64_t block, uint32_t blocks_to_write)
{
ATOSE_msb_uint64_t *address_64;
ATOSE_msb_uint32_t *address_32, *number_32;
ATOSE_msb_uint16_t *number_16;
ATOSE_lsb_uint32_t *bytes_to_transfer;
uint8_t command[31];

/*
	Set up the USB transfer
*/
if (block < 0xFFFFFFFF && blocks_to_write < 0xFFFF)
	{
	/*
		We can do a 32-bit transfer
	*/
	memcpy(command, ATOSE_usb_scsi_write_10, sizeof(command));

	address_32 = (ATOSE_msb_uint32_t *)(command + 17);
	*address_32 = (uint32_t)block;
	number_16 = (ATOSE_msb_uint16_t *)(command + 22);
	*number_16 = (uint16_t)blocks_to_write;
	}
else
	{
	/*
		We do a 64-bit transfer.  This code has not been tested because I don't have a large enough disk
	*/
	memcpy(command, ATOSE_usb_scsi_write_16, sizeof(command));
	address_64 = (ATOSE_msb_uint64_t *)(command + 17);
	*address_64 = block;
	number_32 = (ATOSE_msb_uint32_t *)(command + 25);
	*number_32 = (uint32_t)blocks_to_write;
	}

/*
	Tell the USB how many bytes to send us
*/
bytes_to_transfer = (ATOSE_lsb_uint32_t *)(command + 8);
*bytes_to_transfer = block_size * blocks_to_write;

return perform_transaction(command, buffer, buffer_length);
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::SCSI_SYNCHRONIZE_CACHE()
	----------------------------------------------------
*/
uint32_t ATOSE_host_usb_device_disk::scsi_synchronize_cache(void)
{
if (perform_transaction(ATOSE_usb_scsi_synchoronise_cache_10) != 0)
	return perform_transaction(ATOSE_usb_scsi_synchoronise_cache_16);

return 0;
}

/*
	ATOSE_HOST_USB_DEVICE_DISK::GET_DISK_INQUIRY()
	----------------------------------------------
*/
template <class T> T min(T a, T b) { return a < b ? a : b; }

uint32_t ATOSE_host_usb_device_disk::get_disk_inquiry(void)
{
uint32_t error;
uint8_t *partition_table;
ATOSE_file_control_block *fcb, fcb_space;

uint8_t buffer[block_size];

//debug_print_string("READ DISK BLOCK\r\n");
if ((error = scsi_read(buffer, block_size, 0, 1)) != 0)
	return error;

partition_table = buffer + 0x1BE;
ATOSE_fat fat(this, (uint32_t)(*(ATOSE_lsb_uint32_t *)(partition_table + 0x08)));

debug_print_string("\r\nDirectory\r\nNAME\r\n----\r\n");
fat.dir();
fcb = fat.create(&fcb_space, (uint8_t *)"long file names with spaces are for the.whole.world.txt");
#ifdef NEVER
debug_print_string("OPEN FILE\r\n");
	fcb = fat.open(&fcb_space, (uint8_t *)"B.IN");

	if (fcb == NULL)
		debug_print_string("Cannot open file\r\n");
	else
		{
debug_print_string("OPEN FILE\r\n");

		uint8_t fcb_buffer[fcb->block_size_in_bytes];
		uint8_t into[0x2500];

		debug_print_string("\r\n");
		debug_print_this("first_block        :", fcb->first_block);
		debug_print_this("block_size_in_bytes:", fcb->block_size_in_bytes);
		debug_print_this("file_size_in_bytes :", fcb->file_size_in_bytes);
		debug_print_this("current_block      :", fcb->current_block);
		debug_print_this("file_offset        :", fcb->file_offset);

		fcb->buffer = fcb_buffer;
		debug_print_string("Read\r\n");
		fat.read(fcb, into, 0x2000);
		debug_dump_buffer(into + 0x1000 - 0x20, 0x1000 - 0x20, 0x40);

		debug_print_string("Write\r\n");
		fat.seek(fcb, 0x1000 - 0x20);
		fat.write(fcb, (uint8_t *)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQR", 40);

		memset(into, 0, sizeof(into));
		debug_print_string("Check\r\n");
		fat.seek(fcb, 0);
		fat.read(fcb, into, 0x2000);

		debug_dump_buffer(into + 0x1000 - 0x20, 0x1000 - 0x20, 0x40);

		fat.extend(fcb, 0x10000);
		}
#endif

//scsi_synchronize_cache();
debug_print_string("\r\nDirectory\r\nNAME\r\n----\r\n");
fat.dir();

return error;
}
