/*
	HOST_USB_DEVICE_DISK.C
	----------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD


required SCSI commands
- Inquiry - spc3r23.pdf(p142)
- Request Sense - spc3r23.pdf(p221)
- Test Unit Ready - spc3r23.pdf(p232)
- Read(10) - sbc2r16.pdf(p47)
- Read Capacity(10) - sbc2r16.pdf(p54)
- Write(10) - sbc2r16.pdf(p78)

*/
#include "atose.h"
#include "ascii_str.h"
#include "host_usb_device.h"
#include "host_usb_device_disk.h"
#include "host_usb.h"

#include "usb_disk_command_status_wrapper.h"

#include "usb_standard_interface_descriptor.h"
#include "usb_standard_endpoint_descriptor.h"

static uint8_t ATOSE_usb_scsi_inquiry[31] =          {0x55, 0x53, 0x42, 0x43, 0x00, 0x41, 0x54, 0x00, 0x24, 0x00, 0x00, 0x00, 0x80, 0x00, 0x06, 0x12, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
55 53 42 43 
00 DE 2E 82 
FC 00 00 00 
80 
00 
0A 

23 
00 00 00 00 00 00 00 
FC 
00 00 00 00 00 00 00 
*/

static uint8_t ATOSE_usb_scsi_read_format_capacities[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	0xFC, 0x00, 0x00, 0x00, 	// dCBWDataTransferLength
	0x80, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	0x0A, 						// bCBWCBLength

	0x23, 						// SCSI Command
	0x00, 						// SCSI lun
	0x00,						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI MSB Length
	0xFC, 						// SCSI LSB Length
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	};

static uint8_t ATOSE_usb_scsi_read_capacity_10[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	0x08, 0x00, 0x00, 0x00, 	// dCBWDataTransferLength
	0x80, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	0x0A, 						// bCBWCBLength

	0x25, 						// SCSI Command
	0x00, 						// SCSI RelAddr and Lun
	0x00, 0x00, 0x00, 0x00, 	// SCSI LBA
	0x00,						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI PMI
	0x00						// SCSI Control
	};

static uint8_t ATOSE_usb_scsi_read_capacity_16[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	0x20, 0x00, 0x00, 0x00, 	// dCBWDataTransferLength
	0x80, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	16, 						// bCBWCBLength

	0x9E, 						// SCSI Command
	0x10, 						// SCSI Service Action
	0x00, 0x00, 0x00, 0x00, 	// SCSI LBA Address
	0x00, 0x00, 0x00, 0x00, 	// SCSI LBA Address
	0x00, 0x00, 0x00, 0x20, 	// SCSI Allocation Length	 (send me 32-bytes)
	0x00, 						// SCSI PMI
	0x00						// SCSI Control
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

static uint8_t  __attribute__ ((aligned(64))) ATOSE_usb_scsi_read_10[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	0x00, 0x02, 0x00, 0x00, 	// dCBWDataTransferLength
	0x80, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	0x0A,	 					// bCBWCBLength

	0x28, 						// SCSI Command
	0x00, 						// SCSI flags
	0x00, 					 	// SCSI MSB Address
	0x00, 					 	// SCSI     Address
	0x00, 					 	// SCSI     Address
	0x00, 					 	// SCSI LSB Address
	0x00, 					 	// SCSI Group number
	0x00, 					 	// SCSI MSB Transfer Length
	0x01, 					 	// SCSI LSB Transfer Length
	0x00						// SCSI Control
};

static uint8_t  __attribute__ ((aligned(64))) ATOSE_usb_scsi_test_unit_ready[31] =
	{
	0x55, 0x53, 0x42, 0x43, 	// dCBWSignature
	0x00, 0x41, 0x54, 0x00, 	// dCBWTag
	0x00, 0x00, 0x00, 0x00, 	// dCBWDataTransferLength
	0x00, 						// bmCBWFlags
	0x00, 						// bCBWLUN
	0x06,	 					// bCBWCBLength

	0x00, 						// SCSI Command
	0x00, 						// SCSI LUN
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00, 						// SCSI Reserved
	0x00						// SCSI Control
};


//static uint8_t ATOSE_usb_scsi_test_unit_read[] =   {0x55, 0x53, 0x42, 0x43, 0x00, 0x41, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
uint8_t buffer[1024];
ATOSE_usb_disk_command_status_wrapper *ans;

if ((error = get_max_lun(&logical_units)) == 0)
	{
	/*
		The device return the highest LUN number, we want the number of LUNs
	*/
	logical_units++;

	memset(buffer, 0, sizeof(buffer));

	debug_print_string("set congif\r\n");
	set_configuration(1);



//#ifdef NEVER
	debug_print_string("\r\nINQUIRY\r\n");
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_inquiry, sizeof(ATOSE_usb_scsi_inquiry), endpoint_in, buffer, 0x24);
	buffer[32] = '\0';
	debug_print_string((char *)(buffer + 8));
	//debug_dump_buffer(buffer, 0, 96);
	//debug_print_string("\r\n");

//	void print_info(const char *buffer);
//	print_info((char *)buffer);
//while (1);

	//debug_print_string("REC:\r\n");
	//memset(buffer, 0, sizeof(buffer));
	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);

	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);

	//debug_dump_buffer(buffer, 0, 13);
	//debug_print_string("\r\n");
//#endif
///
///
///
//#ifdef NEVER
	debug_print_string("\r\nREQUEST_SENSE\r\n");
	memset(buffer, 0, sizeof(buffer));
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_request_sense, 31, endpoint_in, buffer, 18);

	debug_print_this("Valid                :", buffer[0] & 0x80);
	debug_print_this("Error Code           :", buffer[0] & 0x7F);
	debug_print_this("ILI                  :", buffer[2] & (1 << 5));
	debug_print_this("Sense Key            :", buffer[2] & 0x0F);
	debug_print_this("Info MSB             :", buffer[3]);
	debug_print_this("Info                 :", buffer[4]);
	debug_print_this("Info                 :", buffer[5]);
	debug_print_this("Info LSB             :", buffer[6]);
	debug_print_this("Additional Sense Len :", buffer[7]);
	debug_print_this("Command Specific MSB :", buffer[8]);
	debug_print_this("Command Specific     :", buffer[9]);
	debug_print_this("Command Specific     :", buffer[10]);
	debug_print_this("Command Specific LSB :", buffer[11]);
	debug_print_this("Additional Sense Code:", buffer[12]);
	debug_print_this("Qualifier            :", buffer[13]);

	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);

	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);
//#endif
//#ifdef NEVER
	memset(buffer, 0, sizeof(buffer));
//	debug_print_string("\r\nREAD CAPACITY 16\r\n");
//	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_capacity_16, 31, endpoint_in, buffer, 32);

//	debug_print_string("\r\nREAD CAPACITY 10\r\n");
//	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_capacity_10, 31, endpoint_in, buffer, 8);

//	debug_print_string("\r\nREAD FORMAT CAPACITY\r\n");
//	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_format_capacities, 31, endpoint_in, buffer, 0xFC);

do
	{
	debug_print_string("\r\nTEST UNIT READY\r\n");
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_test_unit_ready, sizeof(ATOSE_usb_scsi_test_unit_ready), endpoint_in, buffer, 13);
	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);

	if (ans->bCSWStatus == 0x00)
		break;

	debug_print_string("REQUEST_SENSE\r\n");
	memset(buffer, 0, sizeof(buffer));
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_request_sense, 31, endpoint_in, buffer, 18);

	debug_print_this("Valid                :", buffer[0] & 0x80);
	debug_print_this("Error Code           :", buffer[0] & 0x7F);
	debug_print_this("ILI                  :", buffer[2] & (1 << 5));
	debug_print_this("Sense Key            :", buffer[2] & 0x0F);
	debug_print_this("Info MSB             :", buffer[3]);
	debug_print_this("Info                 :", buffer[4]);
	debug_print_this("Info                 :", buffer[5]);
	debug_print_this("Info LSB             :", buffer[6]);
	debug_print_this("Additional Sense Len :", buffer[7]);
	debug_print_this("Command Specific MSB :", buffer[8]);
	debug_print_this("Command Specific     :", buffer[9]);
	debug_print_this("Command Specific     :", buffer[10]);
	debug_print_this("Command Specific LSB :", buffer[11]);
	debug_print_this("Additional Sense Code:", buffer[12]);
	debug_print_this("Qualifier            :", buffer[13]);


	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);
	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);

	debug_print_string("Wait a second");
	ATOSE_atose::get_ATOSE()->cpu.delay_us(1000000);
	}
while (1);
//#endif

///
///
//
	debug_print_string("\r\nREAD CAPACITY 10\r\n");
	memset(buffer, 0, sizeof(buffer));
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_capacity_10, sizeof(ATOSE_usb_scsi_read_capacity_10), endpoint_in, buffer, 8);

	debug_print_string("\r\n");
	debug_dump_buffer(buffer, 0, 8);

	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);

	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);

///
///
///

	debug_print_string("\r\nREAD 6\r\n");
	memset(buffer, 0, sizeof(buffer));
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_read_10, sizeof(ATOSE_usb_scsi_read_10), endpoint_in, buffer, 512);

	debug_print_string("\r\n");
	debug_dump_buffer(buffer, 0, 512);

	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);

	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);


///
///
///
	debug_print_string("\r\nINQUIRY\r\n");
	memset(buffer, 0, sizeof(buffer));
	ehci->send_and_recieve_packet(this, endpoint_out, ATOSE_usb_scsi_inquiry, sizeof(ATOSE_usb_scsi_inquiry), endpoint_in, buffer, 0x24);

	buffer[32] = '\0';
	debug_print_string((char *)(buffer + 8));

	memset(buffer, 0, sizeof(buffer));
	ehci->recieve_packet(this, endpoint_in, buffer, 13);

	ans = (ATOSE_usb_disk_command_status_wrapper *)buffer;
	debug_print_this("\r\nSIG:", ans->dCSWSignature);
	debug_print_this("TAG:", ans->dCSWTag);
	debug_print_this("RES:", ans->dCSWDataResidue);
	debug_print_this("STS:", ans->bCSWStatus);
	}
return error;
}











struct _BraseroScsiInquiry {
	unsigned char type			:5;
	unsigned char qualifier			:3;

	unsigned char reserved0		:7;
	unsigned char rmb				:1;

	unsigned char ansi_ver			:3;
	unsigned char ecma_ver		:3;
	unsigned char iso_ver			:2;

	unsigned char response_format	:4;
	unsigned char reserved1		:1;
	unsigned char norm_aca			:1;
	unsigned char trmtsk			:1;
	unsigned char aerc			:1;

	unsigned char add_len;

	unsigned char reserved2;

	unsigned char addr16			:1;
	unsigned char addr32			:1;
	unsigned char ack_req			:1;
	unsigned char mchngr			:1;
	unsigned char multiP			:1;
	unsigned char vs1				:1;
	unsigned char enc_serv			:1;
	unsigned char reserved3		:1;

	unsigned char vs2				:1;
	unsigned char cmd_queue		:1;
	unsigned char transdis			:1;
	unsigned char linked			:1;
	unsigned char sync			:1;
	unsigned char wbus16			:1;
	unsigned char wbus32			:1;
	unsigned char rel_addr			:1;

	unsigned char vendor			[8];
	unsigned char name			[16];
	unsigned char revision			[4];
};


void print_info(const char *buffer)
{
struct _BraseroScsiInquiry *ob;

ob = (struct _BraseroScsiInquiry *)buffer;

debug_print_this("type           :",ob->type);
debug_print_this("qualifier      :",ob->qualifier);
debug_print_this("reserved0      :",ob->reserved0);
debug_print_this("rmb            :",ob->rmb);
debug_print_this("ansi_ver       :",ob->ansi_ver);
debug_print_this("ecma_ver       :",ob->ecma_ver);
debug_print_this("iso_ver        :",ob->iso_ver);
debug_print_this("response_format:",ob->response_format);
debug_print_this("reserved1      :",ob->reserved1);
debug_print_this("norm_aca       :",ob->norm_aca);
debug_print_this("trmtsk         :",ob->trmtsk);
debug_print_this("aerc           :",ob->aerc);
debug_print_this("add_len        :",ob->add_len);
debug_print_this("reserved2      :",ob->reserved2);
debug_print_this("addr16         :",ob->addr16);
debug_print_this("addr32         :",ob->addr32);
debug_print_this("ack_req        :",ob->ack_req);
debug_print_this("mchngr         :",ob->mchngr);
debug_print_this("multiP         :",ob->multiP);
debug_print_this("vs1            :",ob->vs1);
debug_print_this("enc_serv       :",ob->enc_serv);
debug_print_this("reserved3      :",ob->reserved3);
debug_print_this("vs2            :",ob->vs2);
debug_print_this("cmd_queue      :",ob->cmd_queue);
debug_print_this("transdis       :",ob->transdis);
debug_print_this("linked         :",ob->linked);
debug_print_this("sync           :",ob->sync);
debug_print_this("wbus16         :",ob->wbus16);
debug_print_this("wbus32         :",ob->wbus32);
debug_print_this("rel_addr       :",ob->rel_addr);

char str[32];
memset(str, 0, sizeof(str));

memcpy(str, ob->vendor, 8);
debug_print_string(str);
debug_print_string("\r\n");

memcpy(str, ob->name, 16);
debug_print_string(str);
debug_print_string("\r\n");

memcpy(str, ob->revision, 4);
str[4] = '\0';
debug_print_string(str);
debug_print_string("\r\n");
}
