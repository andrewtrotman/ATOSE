/*
	IMX_RUN.C
	----------
	Copyright (c) 2013 Andrew Trotman, University of Otago
	Open Source under the BSD License.

	This program has three purposes:
	1. load an imximage file into the memory of an i.MX6Q device attached over the USB OTG port and run it
	2. provide a moderate amount of debugging (see help) over the USB OTG port
	3. load and run an ELF file in the USB OTG attached i.MX6Q
*/

#ifdef __APPLE__
	#include <IOKit/hid/IOHIDLib.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <queue>
#else
	#include <Windows.h>

	extern "C"
	{
	#include <hidsdi.h>
	#include <hidpi.h>
	#include <SetupAPI.h>
	}
#endif

#include <stdio.h>
#include <stdlib.h>

/*
	Standard types
	Its necessary to declare these here because they are not provided by Visual C/C++ 9  (Visual Studio 2008)
*/
#ifdef _MSC_VER
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	typedef unsigned long uint32_t;
	typedef long int32_t;
	typedef unsigned long long uint64_t;
	typedef long long int64_t;
#endif
/*
	These are the USB VID and PID of the i.MX6Q
*/
#define IMX_VID 5538			// 0x15A2:Freescale
#define IMX_PID 84			// 0x0054 i.MX6Q

/*
	class IMX_SERIAL_MESSAGE
	------------------------
	Message (USB HID Report) format
*/
#pragma pack(1)
class imx_serial_message
{
public:
	uint8_t report_id;
	uint16_t command_type;
	uint32_t address;			// in MSB-LSB format
	uint8_t format;
	uint32_t data_count;		// in MSB-LSB format
	uint32_t data;
	uint8_t reserved;
} ;
#pragma pack()

/*
	IMX_SERIAL_MESSAGE command types
	--------------------------------
*/
#define READ_REGISTER		0x0101
#define WRITE_REGISTER		0x0202
#define WRITE_FILE			0x0404
#define ERROR_STATUS		0x0505
#define DCD_WRITE			0x0A0A
#define JUMP_ADDRESS		0x0B0B

/*
	IMX_SERIAL_MESSAGE formats
	--------------------------
*/
#define BITS_8             0x08
#define BITS_16            0x10
#define BITS_32            0x20

/*
	ELF stuff
	---------
*/
typedef uint32_t	Elf32_Addr;
typedef uint16_t	Elf32_Half;
typedef uint32_t	Elf32_Off;
typedef int32_t	Elf32_Sword;
typedef uint32_t	Elf32_Word;
typedef uint32_t	Elf32_Size;

#define EI_NIDENT	16	/* Size of e_ident array. */

/*
	class ELF32_EHDR
	----------------
	ELF file header
*/
class Elf32_Ehdr
{
public:
	unsigned char	e_ident[EI_NIDENT];	/* File identification. */
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} ;

/*
	class ELF32_PHDR
	----------------
	ELF program header
*/
class Elf32_Phdr
{
public:
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Size	p_filesz;
	Elf32_Size	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Size	p_align;
} ;


/*
	Constants to do with the IMXIMAGE file foramt
*/
#define IMX_IMAGE_VERSION                  0x40
#define IMX_IMAGE_FILE_HEADER_LENGTH     0x2000

#define IMX_IMAGE_TAG_FILE_HEADER          0xD1
#define IMX_IMAGE_TAG_DCD                  0xD2

#define IMX_IMAGE_DCD_COMMAND_WRITE_DATA	0xCC
#define IMX_IMAGE_DCD_COMMAND_CHECK_DATA	0xCF
#define IMX_IMAGE_DCD_COMMAND_NOP			0xC0
#define IMX_IMAGE_DCD_COMMAND_UNLOCK		0xB2

/*
	class IMX_IMAGE_HEADER
	----------------------
	File header for the IMXIMAGE file format
*/
#pragma pack(1)
class imx_image_header
{
public:
	uint8_t tag;					// see IMX_IMAGE_TAG_xxx
	uint16_t length;				// BigEndian format
	uint8_t version;				// for the i.MX6 this should be either 0x40 or 0x41
} ;
#pragma pack()

/*
	class IMX_IMAGE_IVT
	-------------------
	See page 441-442 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
class imx_image_ivt
{
public:
	imx_image_header header;		// should be: tag=0xD1, length=0x0020, version=0x40
	uint32_t entry;					// Absolute address of the first instruction to execute from the image
	uint32_t reserved1;
	uint32_t dcd;					// Absolute address of the image DCD. The DCD is optional so this field may be set to NULL if no DCD is required
	uint32_t boot_data;				// Absolute address of the Boot Data
	uint32_t self;					// Absolute address of the IVT
	uint32_t csf;					// Absolute address of Command Sequence File (CSF) used by the HAB library
	uint32_t reserved2;
} ;
#pragma pack()

/*
	class IMX_IMAGE_BOOT_DATA
	-------------------------
	See page 442 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
class imx_image_boot_data
{
public:
	uint32_t start;			// Absolute address of the image
	uint32_t length;		// Size of the program image
	uint32_t plugin;		// Plugin flag
} ;
#pragma pack()

/*
	class IMX_IMAGE_DCD_HEADER
	--------------------------
	See page 443-444 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
class imx_image_dcd_header
{
public:
	uint8_t tag;			// the DCD command  (0xCC for a write)
	uint16_t length;		// BigEndian format
	uint8_t parameter;		// This is command specific, but imximage (in  u-boot) appears to only use 0x02 (i.e. 16-bits) for the write command
} ;
#pragma pack()

/*
	class IMX_IMAGE_DCD_WRITE_TUPLE
	-------------------------------
	See page 443-444 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
#pragma pack(1)
class imx_image_dcd_write_tuple
{
public:
	uint32_t address;		// write to this address
	uint32_t value;			// write this value
} ;
#pragma pack()

/*
	class IMX_IMAGE_DCD_WRITE
	-------------------------
*/
#pragma pack(1)
class imx_image_dcd_write
{
public:
	imx_image_dcd_header header;			// header
#ifdef _MSC_VER
	#pragma warning(disable:4200)
#endif
	imx_image_dcd_write_tuple action[];		// and the list of actions
} ;
#pragma pack()

/*
	class IMX_IMAGE_DCD
	-------------------
*/
#pragma pack(1)
class imx_image_dcd
{
public:
	imx_image_header header;				// this is the header of the entire DCD
	imx_image_dcd_write command;			// the first command in the DCD (should probably be a union based on the header's tag)
} ;
#pragma pack()

/*
	Verbose mode is more explicit about the communications thats going on
*/
long verbose;

#ifdef __APPLE__
	/*
		Methods to mimic the Windows HID interface on the Mac
	*/

	typedef uint32_t DWORD;
	#define HANDLE IOHIDDeviceRef
	#define _strtoui64 strtoull

	typedef union _LARGE_INTEGER
	{
	struct
		{
		uint32_t LowPart;
		int32_t  HighPart;
		};
	struct
		{
		uint32_t LowPart;
		int32_t  HighPart;
		} u;
	int64_t QuadPart;
	} LARGE_INTEGER, *PLARGE_INTEGER;

	uint8_t mac_hid_buffer[1024 * 1024];

	class mac_hid_message_object
	{
	public:
		IOReturn result;
		IOHIDReportType type;
		uint32_t reportID;
		uint8_t *report;
		CFIndex reportLength;
	} ;

	std::queue<mac_hid_message_object *> message_queue;

	/*
		CALLBACK()
		----------
	*/
	void callback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength)
	{
	mac_hid_message_object *object;

	if (verbose)
		printf("Got a report of ID:%u (len:%ld)\n", reportID, ((long)reportLength));

	object = new mac_hid_message_object;
	object->result = result;
	object->type = type;
	object->reportID = reportID;
	object->reportLength =reportLength;
	object->report = new uint8_t [reportLength];
	memcpy(object->report, report, reportLength);

	message_queue.push(object);
	}

	/*
		HID_INIT()
		----------
	*/
	void hid_init(IOHIDDeviceRef hDevice)
	{
	IOHIDDeviceRegisterInputReportCallback(hDevice, mac_hid_buffer, sizeof(mac_hid_buffer), callback, NULL);
	}

	template <class T> T min(T a, T b) { return a < b ? a : b; }

	/*
		READFILE()
		----------
	*/
	long ReadFile(IOHIDDeviceRef hDevice, void *recieve_buffer, uint32_t to_read, uint32_t *did_read, void *ignore)
	{
	mac_hid_message_object *object;

	do
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
	while (message_queue.empty());

	object = message_queue.front();
	message_queue.pop();
	memcpy(recieve_buffer, object->report, *did_read = min(to_read, (uint32_t)object->reportLength));

	delete [] object->report;
	delete object;

	return 1;
	}

	/*
		WRITE_FILE()
		------------
	*/
	long WriteFile(IOHIDDeviceRef hDevice, void *transmit_buffer, uint32_t transmit_buffer_length, uint32_t *dwBytes, void *ignore)
	{
	IOHIDDeviceSetReport(hDevice, kIOHIDReportTypeOutput, *(uint8_t *)transmit_buffer, ((uint8_t *)transmit_buffer), transmit_buffer_length);
	*dwBytes = transmit_buffer_length;

	return 1;
	}

	/*
		HIDD_SETOUTPUTREPORT()
		----------------------
	*/
	long HidD_SetOutputReport(IOHIDDeviceRef hDevice, void *message, unsigned long message_length)
	{
	return IOHIDDeviceSetReport(hDevice, kIOHIDReportTypeOutput, *((uint8_t *)message), (uint8_t *)message, message_length) == kIOReturnSuccess;
	}

	/*
		CREATE_DICTIONARY()
		-------------------
		Create a dictionary matching VID and PID
	*/
	static CFMutableDictionaryRef create_dictionary(UInt32 product_id, UInt32 vendor_id)
	{
	CFMutableDictionaryRef result;
	CFNumberRef number;
	/*
		Create a dictionary
	*/
	result = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	/*
		Match the product ID
	*/
	number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor_id);
	CFDictionarySetValue(result, CFSTR(kIOHIDVendorIDKey), number);
	CFRelease(number);

	/*
		Match the vendor ID
	*/
	number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product_id);
	CFDictionarySetValue(result, CFSTR(kIOHIDProductIDKey), number);
	CFRelease(number);

	return result;
	}

#endif

/*
	The recieve and transmit buffers are allocated at runtime based on the
	capabilities information returned by the USB HID device
*/
unsigned char *recieve_buffer, *transmit_buffer;
uint32_t recieve_buffer_length, transmit_buffer_length;

/*
	Due to a bug in the i.MX6Q ROMs we need a buffer the same size as the transmit buffer
	to hold the contents of the last one kilobyte of RAM we're going to write into when
	we do a write_file.
*/
unsigned char *transmit_double_buffer;

/*
	This program can also be used to download an ELF file to the i.MX6Q and execute it. These globals store the file
	the its length for any file we're going to upload (or NULL on none) and length of that file.
*/
char *elf_file = NULL;
char *elf_filename = NULL;
long long elf_file_length = 0;

/*
	In Windows there are two ways to communicate with a HID device, one is using HidD_SetOutputReport() and HidD_GetInputReport()
	while the other is to use WriteFile() and ReadFile().  It turns out they are not equivelant.  the HidD methods send the
	messages to the control endpoint (endpoint 0) whereas the File methods use the interrupt and bulk endpoints.  It also appears
	as though when you call HidD_GetInputReport() the ID of the report you want is transferred to the device as part of the
	request (but we don't use that method here).

	In the case of the i.MX6Q, we want to send a message to the control end point and then read what ever it sends us back.
	This means we must use HidD_SetOutputReport() to tell the i.MX6Q what to do and then read the reply using ReadFile(). If we
	call HidD_GetInputReport() after HidD_SetOutputReport() it appears to pass the ID of the requestd report to the i.MX6Q
	which then fails.

	The "Serial Downloader" on the i.MX6 supports 4 reports (see Table 8-41 on page 450 of "i.MX 6Dual/6Quad Applications
	Processor Reference Manual Rev. 0, 11/2012").  They are:

	Report 1 (host to device, Control Endpoint)
		17 bytes SDP command from host to device
	Report 2 (host to device, Control Endpoint)
		Up to 1025 bytes Data associated with report 1 SDP command
	Report 3 (device to host, Interrupt Endpoint)
		5 bytes HAB security configuration (0x12343412 in closed mode, 0x56787856 in open mode)
	Report 4 (device to host, Interrupt Endpoint)
		Up to 65 bytes Data in response to SDP command sent in Report 1

	The "Serial Download Protocol" is described on page 451-457 of "i.MX 6Dual/6Quad Applications Processor Reference Manual
	Rev. 0, 11/2012"
*/

/*
	INTEL_TO_ARM()
	--------------
*/
uint32_t intel_to_arm(uint32_t value)
{
union i_to_b
{
uint32_t number;
struct { unsigned char one, two, three, four; } byte;
} in, out;

in.number = value;
out.byte.one = in.byte.four;
out.byte.two = in.byte.three;
out.byte.three = in.byte.two;
out.byte.four = in.byte.one;

return out.number;
}

/*
	ARM_TO_INTEL()
	--------------
*/
uint32_t arm_to_intel(uint32_t value)
{
return intel_to_arm(value);			// call intel_to_arm() as the result is the same (endian "swap");
}

/*
	ARM_TO_INTEL()
	--------------
*/
uint16_t arm_to_intel(uint16_t value)
{
union i_to_b
{
uint16_t number;
struct { unsigned char one, two; } byte;
} in, out;

in.number = value;
out.byte.one = in.byte.two;
out.byte.two = in.byte.one;

return out.number;
}

/*
	DUMP_BUFFER()
	-------------
*/
void dump_buffer(unsigned char *buffer, uint64_t address, uint64_t bytes)
{
uint64_t remaining, width, column;

remaining = bytes;
while (remaining > 0)
	{
	printf("%04X: ", (uint32_t)address);

	width = remaining > 0x10 ? 0x10 : remaining;

	for (column = 0; column < width; column++)
		printf("%02X ", buffer[column]);

	for (; column < 0x10; column++)
		printf("   ");

	printf(" ");
	for (column = 0; column < width; column++)
		printf("%c", isprint(buffer[column]) ? buffer[column] : '.');

	puts("");
	buffer += width;
	address += width;
	remaining -= width;
	}
}


/*
	IMX_GET_SECURITY_DESCRIPTOR()
	-----------------------------
	Read a Report 3 from the i.MX6 and make sure the decice is open. The report 3 is
	5 bytes long and will be either 0x0312343412 on a "closed" system or 0x0356787856
	on an open system.  We consider a closed system as failure,

	return 0 on failure or closed system, 1 on success (open)
*/
uint32_t imx_get_security_descriptor(HANDLE hDevice)
{
unsigned char open_system[] = {0x03, 0x56, 0x78, 0x78, 0x56};
DWORD dwBytes = 0;

if (!ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))
	return 0;

if (memcmp(recieve_buffer, open_system, 5) == 0)
	return 1;

printf("Security Descriptor Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);

return 0;
}

/*
	IMX_READ_REGISTER()
	-------------------
	See Page 452 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	I send a Report 1
	i.MX6Q sends a Report 3 (security descriptor: 0x12343412 in closed mode or 0x56787856 in open mode)
	i.MX6Q sends a number of Report 4 (data in response)

	This method returns the number of bytes read or 0 on failure
*/
uint32_t imx_read_register(HANDLE hDevice, uint32_t address, uint32_t bytes, unsigned char *into)
{
imx_serial_message message;
DWORD dwBytes = 0;
long long remaining;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = READ_REGISTER;
message.address = intel_to_arm(address);
message.format = BITS_8;
message.data_count = intel_to_arm(bytes);

/*
	Transmit to the board and get the response back
*/
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))
	{
	/*
		Recieve a security report (Report 3)
	*/
	if (imx_get_security_descriptor(hDevice))
		{
		/*
			Recieve as many bytes as this method was asked to retrieve, but in 65-byte packets (on the i.MX6Q).
			We consequently need to joint all these together to build the answer
		*/
		remaining = bytes;
		while (remaining > 0)
			{
			if (!ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))
				return 0;

			dwBytes -= 1;	// skip over the report ID
			memcpy(into, recieve_buffer + 1, (size_t)(dwBytes > remaining ? remaining : dwBytes));

			into += dwBytes > remaining ? remaining : dwBytes;
			remaining -= dwBytes;
			}
		}
	return bytes;		// success
	}

return 0;
}

/*
	IMX_WRITE_REGISTER()
	--------------------
	See Page 453 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	I send a Report 1
	i.MX6Q sends a Report 3 (security descriptor: 0x12343412 in closed mode or 0x56787856 in open mode)
	i.MX6Q sends a Report 4 (response, 0x128A8A12 on success)

	Because the report 1 includes at most 1 32-bit integer for transmission, this method is no use for
	writing into long consequitive blocks of RAM.

	This method returns 1 on success or 0 on failure
*/
uint32_t imx_write_register(HANDLE hDevice, uint32_t address, uint32_t value, uint8_t data_size = BITS_8)
{
unsigned char success[] = {0x04, 0x12, 0x8A, 0x8A, 0x12};
imx_serial_message message;
DWORD dwBytes = 0;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = WRITE_REGISTER;
message.address = intel_to_arm(address);
message.format = data_size;
message.data_count = intel_to_arm((uint32_t)(data_size == BITS_8 ? 1 : data_size == BITS_16 ? 2 : 4));
message.data = intel_to_arm(value);

/*
	Communicate with the i.MX6Q
*/

if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))								// transmit Report 1
	if (imx_get_security_descriptor(hDevice))												// recieve Report 3
		if (ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))		// recieve Report 4
			if (memcmp(recieve_buffer, success, 5) == 0)									// check the status code
				return 1;
			else
				printf("Write Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);

return 0;
}

/*
	IMX_BLOCK_WRITE()
	-----------------
	See page 453-454 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	I send a Report 1
	I optionally send a Report 1 Error Status command (ignored)
	I send data as a series of 1024 byte Report 2 (data) chunks
	i.MX6Q sends a Report 3 (security descriptor: 0x12343412 in closed mode or 0x56787856 in open mode)
	i.MX6Q sends a Report 4 (4 byte error status 0x88888888 on success)

	There appears to be a bug in the i.MX6Q implementation of the Serial Download Protocol (SDP). Although its
	posible to specify the number of bytes that are to be sent, the i.MX6Q actually rounds this up to the nearest
	whole kilobyte and in doing so trashes what ever is in the memory between the end of the short final packet
	and the end of that given memory block.  I suspect its also, therefore, impossible to write into some memory
	locations as they will cause a buffer-overwrite into what could be catestrophic locations.

	To get around this as much as possible its necessary to read the last kilobyte of memory into the transmit
	buffer before writing the contents of the file into it and sending it back to the i.MX6Q!
*/
uint32_t imx_block_write(HANDLE hDevice, uint32_t address, uint32_t bytes, unsigned char *data, uint16_t method, unsigned char *success)
{
imx_serial_message message;
DWORD dwBytes = 0;
unsigned char *from;
uint32_t remaining;
uint32_t current_buffer_length;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = method;
message.address = intel_to_arm(address);
message.data_count = intel_to_arm(bytes);

/*
	Due to a bug in the i.MX6Q ROM we need to read the last kilobyte from the i.MX6Q into the transmit
	buffer, then write our final packet over it and send that packet back.
*/
if (verbose)
	puts("READ FINAL BLOCK");
imx_read_register(hDevice, address + (bytes - (bytes % transmit_buffer_length)), (transmit_buffer_length - 1), transmit_double_buffer);

/*
	Transmit to the board and get the response back
*/
if (verbose)
	puts("SEND BLOCK WRITE REPORT");
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))								// transmit Report 1
	{
	/*
		Send the data
	*/
	remaining = bytes;
	from = data;
	while (from < data + bytes)
		{
		/*
			The first byte to send is the Report ID (in this case 0x02)
			The remainder of the buffer is bytes to send
		*/
		*transmit_buffer = 0x02;
		current_buffer_length = remaining > transmit_buffer_length - 1 ? transmit_buffer_length - 1 : remaining;
		if (current_buffer_length < transmit_buffer_length - 1)
			memcpy(transmit_buffer + 1, transmit_double_buffer, transmit_buffer_length - 1);	// final packet (hack for i.MX6Q ROM bug)
		memcpy(transmit_buffer + 1, from, current_buffer_length);

		/*
			Always transmit a full buffer worth of data (even if it is short)
		*/
		if (verbose)
			puts("SEND BLOCK DATA");

		if (!WriteFile(hDevice, transmit_buffer, transmit_buffer_length, &dwBytes, NULL))
			return 0;

		remaining -= current_buffer_length;
		from += current_buffer_length;
		}

	/*
		Get descriptors back
	*/
	if (verbose)
		puts("GET SECRITY DESCRIPTOR");
	if (imx_get_security_descriptor(hDevice))												// recieve Report 3 (security descriptor)
		{
		if (verbose)
			puts("GET REPORT OF TYPE 4");

		if (ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))		// recieve Report 4
			if (memcmp(recieve_buffer, success, 5) == 0)									// check the status code
				return 1;
			else
				printf("Load File Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);
		}
	}

return 0;
}

/*
	IMX_WRITE_FILE()
	----------------
	See pages 453-454 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	This method is a stub that just sets the message type to WRITE_FILE and then calls imx_block_write
*/
uint32_t imx_write_file(HANDLE hDevice, uint32_t address, uint32_t bytes, unsigned char *data)
{
unsigned char success[] = {0x04, 0x88, 0x88, 0x88, 0x88};

return imx_block_write(hDevice, address, bytes, data, WRITE_FILE, success);
}

/*
	IMX_DCD_WRITE()
	---------------
	See pages 455-456 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	This method is a stub that just sets the message type to DCD_WRITE and then calls imx_block_write

	See pages 442-447 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012" for
	information on the format of a DCD file

	Note: The address is the load location of the DCD before it is executed and the bytes is the
	length of the entire DCD including all headers.
*/
uint32_t imx_dcd_write(HANDLE hDevice, uint32_t address, uint32_t bytes, unsigned char *data)
{
unsigned char success[] = {0x04, 0x12, 0x8A, 0x8A, 0x12};

return imx_block_write(hDevice, address, bytes, data, DCD_WRITE, success);
}

/*
	IMX_JUMP_ADDRESS()
	------------------
	See page 456 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	I send a Report 1
	i.MX6Q sends a Report 3 (security descriptor: 0x12343412 in closed mode or 0x56787856 in open mode)
	i.MX6Q, optionally (on error), sends a Report 4 (4 byte HAB error status)

	return 1 on success 0 on failure

	After some experimentation, it turns out that the JUMP_ADDRESS command does *not* simply load PC with
	a memory location to to a branch.  It actually instructs the ROM to find an imximage at the given location
	and to run that.  This eventually results in a branch to the imximage->entry vector.  Since that vector
	is 4 bytes after the start of the file its the same as (pc = *(address + 4)).  In order to make it possible
	to run ELF files we put the whole header there rather than just the jump address (because it needs a
	null DCD and so on).
*/
uint32_t imx_jump_address(HANDLE hDevice, uint32_t address)
{
imx_serial_message message;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = JUMP_ADDRESS;
message.address = intel_to_arm(address);

/*
	Transmit to the board and get the response back
*/
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))								// transmit Report 1
	if (imx_get_security_descriptor(hDevice))													   // recieve Report 3 (security descriptor)
		{
		/*
			At present this isn't returning (because the board is running) so we ignore the possible Report 4
		*/
#ifdef NEVER
		DWORD dwBytes = 0;
		/*
			We expect this to time out if the device is functioning correctly.  If we get a reply
			then the result is a 4-byte HAB code.
		*/
		memset(recieve_buffer, 0, recieve_buffer_length);
		if (!ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))		// recieve Report 4
			return 1;		// failure to read is a success (because the board is busy)

		printf("Jump Address Error:HIDReport%02X = %02X%02X%02X%02X\n", recieve_buffer[0], recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);
		return 0;		// but the HAB code is in recieve_buffer[1]..recieve_buffer[4]
#endif
		}
return 0;
}

/*
	IMX_ERROR_STATUS()
	------------------
	See page 454-455 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	I send a Report 1
	i.MX6Q sends a Report 3 (security descriptor: 0x12343412 in closed mode or 0x56787856 in open mode)
	i.MX6Q sends a Report 4 (4 byte error status)

	The list of error statuses is near impossible to find!  After some searching it appears as though the
	code that is returned is 4 copies of the HAB (High Accuracy Boot) error status.  Freescale provide a
	list of those status numbers thus (see page 28 of "Secure Boot on i.MX50, i.MX53, and i.MX 6 Series using
	HABv4 Rev. 0, 10/2012")

	typedef enum hab_status
	{
	HAB_STS_ANY = 0x00,
	HAB_FAILURE = 0x33,
	HAB_WARNING = 0x69,
	HAB_SUCCESS = 0xf0
	} hab_status_t;

	returns the HAB status code (or 0 on failure).  Yes, 0 has two meanings (but at least they both mean fail)
*/
uint32_t imx_error_status(HANDLE hDevice)
{
imx_serial_message message;
DWORD dwBytes = 0;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = ERROR_STATUS;

/*
	Communicate with the i.MX6Q
*/
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))								// transmit Report 1
	if (imx_get_security_descriptor(hDevice))												// recieve Report 3
		if (ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))		// recieve Report 4
			return *((uint32_t *)(recieve_buffer + 1));										// return the status code

return 0;
}

/*
	ELF_LOAD()
	----------
	Given a pointer to an ELF file, send it to the i.MX6Q board and tell the board to execute it.
	We don't do any checking here because we already have control over the board pre-boot and so
	it really doesn't matter what we download into it.

	Return the program entry point on success or -1 (0xFFFFFFFF) on error.
*/
uint32_t elf_load(HANDLE hDevice, char *file, uint32_t length)
{
Elf32_Ehdr *header;				// the ELF file header
uint32_t header_offset;			// location (in the file) of the first header
uint32_t header_num;				// number of headers
Elf32_Phdr *current_header;		// the current header
uint32_t which;					// which header we're currenty looking at
char *buffer;						// Used to create long string of zeros to clear the various sections of memory

/*
	Get a pointer to the elf header
*/
header = (Elf32_Ehdr *)file;

/*
	Now we move on to the ELF program header
*/
header_offset = header->e_phoff;
header_num = header->e_phnum;

/*
	Find the segments.  There should be at most three, .text, .data and .bss
*/
current_header = (Elf32_Phdr *)(file + header_offset);
for (which = 0; which < header_num; which++)
	{
	/*
		For each section we'll first first zero out that memory then we'll copy the contents of the
		section into it. That way we can guarantee to get zero's into the BSS and DATA sections, and
		any necessary zeroes into the code section.
	*/
	buffer = (char *)malloc(current_header->p_memsz);
	memset(buffer, 0, current_header->p_memsz);

	if (verbose)
		puts("CLEAR RAM");

	imx_write_file(hDevice, current_header->p_vaddr, current_header->p_memsz, (unsigned char *)buffer);

	if (verbose)
		puts("TRANSMIT FILE");

	imx_write_file(hDevice, current_header->p_vaddr, current_header->p_filesz, (unsigned char *)(file + current_header->p_offset));
	free(buffer);
	current_header++;
	}

return header->e_entry;
}

/*
	IMXIMAGE_LOAD()
	---------------
	Given a pointer to a valid imximage file, load it into the memory and execute it.  For details
	of the imxfile file format see Section 8.6 starting on page 440 of "i.MX 6Dual/6Quad Applications
	Processor Reference Manual Rev. 0, 11/2012"
*/
uint32_t imximage_load(HANDLE hDevice, char *raw_file, uint32_t raw_file_length)
{
imx_image_ivt *file;
imx_image_dcd *dcd;
char *command_end, *command_start, *dcd_end;
imx_image_boot_data *boot_data;

/*
	Get the file header
*/
file = (imx_image_ivt *)raw_file;

/*
	Get the boot_data header
*/
if (file->boot_data != 0)
	boot_data = (imx_image_boot_data *)(raw_file + file->boot_data - file->self);

/*
	Process the DCD
*/
if (file->dcd != 0)
	{
	dcd = (imx_image_dcd *)(raw_file + file->dcd - file->self);

	dcd_end = ((char *)dcd) + arm_to_intel(dcd->header.length);
	command_start = command_end = (char *)&(dcd->command);
	while(command_start < dcd_end)
		{
		switch (((imx_image_dcd_write *)command_start)->header.tag)
			{
			case IMX_IMAGE_DCD_COMMAND_WRITE_DATA:
				{
				imx_image_dcd_write_tuple *current;

				command_start += sizeof(imx_image_dcd_header);
				command_end += arm_to_intel(dcd->command.header.length);
				for (current = (imx_image_dcd_write_tuple *)command_start; (char *)current < command_end; current++)
					{
					if (verbose)
						printf("   *((uint32_t *)0x%08X) = 0x%08X;\n", arm_to_intel(current->address), arm_to_intel(current->value));
					imx_write_register(hDevice, arm_to_intel(current->address), arm_to_intel(current->value), BITS_32);
					}
				break;
				}
			default:
				puts("Unrecognised DCD command");
				break;
			}
		command_start = command_end;
		}
	puts("");
	}

/*
	Now the application data (i.e. the binary we're going to run)
*/
if (file->entry != 0)
	{
	file->dcd = 0;				// We've already transmitted the DCD to pretend it doesn't exist

	if (verbose)
		printf("Transmit the %d byte file (0x%02X 0x%02X 0x%02X 0x%02X...) to the board (address: 0x%08X)\n", raw_file_length, (unsigned char)raw_file[0], (unsigned char)raw_file[1], (unsigned char)raw_file[2], (unsigned char)raw_file[3], file->self);
	imx_write_file(hDevice, file->self, raw_file_length, (unsigned char *)raw_file);
	}

return file->self;
}

/*
	HELP()
	------
*/
void help(void)
{
puts("COMMAND LINE ARGUMENTS");
puts("----------------------");
puts("-?                            : display this message");
puts("-verbose                      : display each command as its performed (helps with debugging)");
puts("<elf_filename>                : find the first appropriate device, load <elf_filename> onto it, run it, and terminate");
puts("<imx_filename>                : find the first appropriate device, load <imx_filename> onto it, run it, and terminate");
puts("                              : if no <elf_filename> is given the enter interactive mode and accept COMMANDs from below");
puts("");
puts("COMMANDS");
puts("--------");
puts("?                             : display this message");
puts("<address>                     : read <bytes> from <address>.  Specify in Hex");
puts("<address>: <byte> ... <byte>  : write <byte> stream to memory starting at <address>.  Specify in Hex");
puts("<address>g                    : branch to <address> (e.g. C600g).  Specify in Hex");
puts("help                          : display this message");
puts("info                          : display this message");
puts("load <address> <filename>     : load the contents of <filename> at <address>. Specify in Hex. <filename> is *not* and ELF file");
puts("load_dcd <address> <filename> : load the contents of Device Configuration Data (DCC) <filename> at <address>. Specify in Hex");
puts("man                           : display this message");
puts("quit                          : exit this program");
puts("read <address> <bytes>        : read <bytes> from <address>.  Specify in Hex");
puts("run <address>                 : branch to <address>.  Specify in Hex");
puts("status                        : fetch and print current (HAD) error status. (0xF0F0F0F0 is success)");
puts("verbose                       : toggle (on->off, off->on) verbose mode (see above)");
}

/*
	STRIP_SPACE_INPLACE()
	---------------------
	remove spaces from the start and end of a line
*/
char *strip_space_inplace(char *source)
{
char *end, *start = source;

while (isspace(*start))
	start++;

if (start > source)
	memmove(source, start, strlen(start) + 1);		// copy the '\0'

end = source + strlen(source) - 1;
while ((end >= source) && (isspace(*end)))
	*end-- = '\0';

return source;
}


/*

	READ_FILE_64()
	--------------
*/
#ifdef _MSC_VER
	int read_file_64(HANDLE fp, void *destination, long long bytes_to_read)
	{
	unsigned char *from;
	const DWORD chunk_size = ((long long) 1 << (8 * sizeof(DWORD))) - 1;		// the compiler should convert this into a constant
	DWORD got_in_one_read;

	from = (unsigned char *)destination;
	while (bytes_to_read > chunk_size)
		{
		if (ReadFile(fp, from, chunk_size, &got_in_one_read, NULL) == 0)
			return 0;
		bytes_to_read -= chunk_size;
		from += chunk_size;
		}
	if (bytes_to_read > 0)			// catches a call to read 0 bytes 
		if (ReadFile(fp, from, (DWORD)bytes_to_read, &got_in_one_read, NULL) == 0)
			{
			DWORD error_code = GetLastError();			// put a break point on this in the debugger to work out what went wrong.

			return 0;
			}

	return 1;
	}
#else
	int read_file_64(FILE *fp, void *destination, long long bytes_to_read)
	{
	static long long chunk_size = 1 * 1024 * 1024 * 1024;
	long long in_one_go = chunk_size < bytes_to_read ? chunk_size : bytes_to_read;
	long long bytes_read = 0;
	long long bytes_left_to_read = bytes_to_read;
	char *dest = (char *)destination;

	while (bytes_read < bytes_to_read)
		{
		if (bytes_left_to_read < in_one_go)
			in_one_go = bytes_left_to_read;

		bytes_read += fread(dest + bytes_read, 1, in_one_go, fp);
		bytes_left_to_read = bytes_to_read - bytes_read;

		if (ferror(fp))
			return 0;
		}
	return bytes_read == bytes_to_read; // will return 0 (fail) or 1 (success)
	}

#endif


/*
	READ_ENTIRE_FILE()
	------------------
*/
char *read_entire_file(char *filename, long long *file_length)
{
long long unused;
char *block = NULL;
#ifdef _MSC_VER
	HANDLE fp;
	LARGE_INTEGER details;
	#if defined(UNICODE) || defined(_UNICODE)
		LPCWSTR true_filename = (LPCWSTR)filename;					// If we're in UNICODE land then the filename is actually a wide-string
	#else
		char *true_filename = filename;									// else we're in ASCII land and so the filename is a c-string
	#endif
#else
	FILE *fp;
	struct stat details;
#endif

if (filename == NULL)
	return NULL;

if (file_length == NULL)
	file_length = &unused;

#ifdef _MSC_VER
	fp = CreateFile(true_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fp == INVALID_HANDLE_VALUE)
		{
		DWORD error_code = GetLastError();			// put a break point on this in the debugger to work out what went wrong.
		return NULL;
		}

	if (GetFileSizeEx(fp, &details) != 0)
		if ((*file_length = details.QuadPart) != 0)
			if ((block = (char *)malloc((size_t)(details.QuadPart + 1))) != NULL)		// +1 for the '\0' on the end
				{
				if (read_file_64(fp, block, details.QuadPart) != 0)
					block[details.QuadPart] = '\0';
				else
					{
					free(block);
					block = NULL;
					}
				}

	CloseHandle(fp);
#else
	if ((fp = fopen(filename, "rb")) == NULL)
		return NULL;

	if (fstat(fileno(fp), &details) == 0)
		if ((*file_length = details.st_size) != 0)
			if ((block = (char *)malloc((size_t)(details.st_size + 1))) != NULL)		// +1 for the '\0' on the end
				if (read_file_64(fp, block, details.st_size) != 0)
					block[details.st_size] = '\0';
				else
					{
					free(block);
					block = NULL;
					}
	fclose(fp);
#endif

return block;
}

/*
	GO_BE_A_MONITOR()
	-----------------
*/
void go_be_a_monitor(HANDLE hDevice)
{
char command[1024];
static unsigned char into[1024 * 1024];
uint64_t address = 0x00000000, bytes_to_read = 0x0F, byte, write_address;
int64_t bytes_to_send = 0;
char *next, *data;
uint32_t status;

do
	{
	printf("]");
	fgets(command, sizeof(command), stdin);
	strip_space_inplace(command);

	if (strcmp(command, "quit") == 0)
		break;
	else if (strcmp(command, "help") == 0)
		help();
	else if (strcmp(command, "info") == 0)
		help();
	else if (strcmp(command, "man") == 0)
		help();
	else if (strcmp(command, "?") == 0)
		help();
	else if (strcmp(command, "verbose") == 0)
		{
		verbose = !verbose;
		printf("Verbose mode %s\n", verbose ? "on" : "off");
		}
	else if (strncmp(command, "read ", 5) == 0)
		{
		sscanf(command + 5, "%llX %llX", &address, &bytes_to_read);
		if (verbose)
			printf("Read %llX (dec:%lld) bytes from %08llX (dec:%lld)\n", bytes_to_read, bytes_to_read, address, address);
		if (bytes_to_read <= 0)
			bytes_to_read = 1;
		else if (bytes_to_read > sizeof(into))
			printf("Cannot dump more than %llX (dec:%lld) bytes in one read\n", (long long)sizeof(into), (long long)sizeof(into));
		else
			{
			if (imx_read_register(hDevice, (uint32_t)address, (uint32_t)bytes_to_read, into))
				dump_buffer(into, address, bytes_to_read);
			else
				puts("Read failed");
			address += bytes_to_read;
			}
		}
	else if (strncmp(command, "run ", 4) == 0)
		{
		sscanf(command + 4, "%llX", &address);
		if (verbose)
			printf("Run %08llX (dec:%lld)\n", address, address);
		if (!imx_jump_address(hDevice, (uint32_t)address))
			puts("Run failed");
		}
	else if (isdigit(*command))
		{
		address = _strtoui64((char *)command, &next, 16);
		if (*next == 'g')		// as in C600g (branch to 0xC600)
			{
			if (verbose)
				printf("Run %08llX (dec:%lld)\n", address, address);
			if (!imx_jump_address(hDevice, (uint32_t)address))
				puts("Run failed");
			}
		else if (*next == '\0')		// list the next 16 bytes from the given address
			{
			bytes_to_read = 0x10;
			if (verbose)
				printf("Read %llX (dec:%lld) bytes from %08llX (dec:%lld)\n", bytes_to_read, bytes_to_read, address, address);
			if (imx_read_register(hDevice, (uint32_t)address, (uint32_t)bytes_to_read, into))
				dump_buffer(into, address, bytes_to_read);
			else
				puts("Read failed");
			address += bytes_to_read;
			}
		else if (*next == ':' || (*next == ' ' && *(next + 1) == ':'))	// else we're a write operation
			{
			write_address = address;
			while (*next != '\0')
				{
				next++;		// move over the char that caused the last parse to fail (i,e, the space).
				byte = _strtoui64(next, &next, 16);
				byte = (unsigned char)byte;
				if (verbose)
					printf("%08llX <- %02llX\n", write_address, byte);
				if (!imx_write_register(hDevice, (uint32_t)write_address, (uint8_t)byte))
					puts("Write failed");
				write_address++;
				}
			}
		else
			puts("Syntax Error");
		}
	else if (strcmp(command, "status") == 0)
		{
		if (verbose)
			puts("Get Status");
		status = imx_error_status(hDevice);
		printf("%08X\n", status);
		}
	else if (strncmp(command, "load ", 5) == 0)
		{
		address = _strtoui64((char *)command + 4, &next, 16);
		strip_space_inplace(next);
		if ((data = read_entire_file(next, &bytes_to_send)) == NULL)
			printf("Cannot read contents of file '%s'\n", next);
		else
			{
			if (verbose)
				{
				printf("Load the contents of '%s' (%lld bytes) to address %08llX (dec:%lld) ", next, bytes_to_send, address, address);
				if (bytes_to_send >= 4)
					printf("(starting: %02X %02X %02X %02X...)", data[0], data[1], data[2], data[3]);
				puts("");
				}
			imx_write_file(hDevice, (uint32_t)address, (uint32_t)bytes_to_send, (unsigned char *)data);
			free(data);
			}
		}
	else if (strncmp(command, "load_dcd ", 9) == 0)
		{
		address = _strtoui64((char *)command + 8, &next, 16);
		strip_space_inplace(next);
		data = read_entire_file(next, &bytes_to_send);
		if (verbose)
			{
			printf("Load the contents of '%s' (%lld bytes) to address %08llX (dec:%lld) ", next, bytes_to_send, address, address);
			if (bytes_to_send >= 4)
				printf("(starting: %02X %02X %02X %02X...)", data[0], data[1], data[2], data[3]);
			puts("");
			}
		imx_dcd_write(hDevice, (uint32_t)address, (uint32_t)bytes_to_send, (unsigned char *)data);
		free(data);
		}
	else if (*command == '\0')
		{
		bytes_to_read = 0x10;
		if (verbose)
			printf("Read %llX (dec:%lld) bytes from %08llX (dec:%lld)\n", bytes_to_read, bytes_to_read, address, address);
		if (imx_read_register(hDevice, (uint32_t)address, (uint32_t)bytes_to_read, into))
			dump_buffer(into, address, bytes_to_read);
		else
			puts("Read failed");
		address += bytes_to_read;
		}
	else
		puts("Syntax Error");
	}
while (*command != 'q');
}

/*
	HID_MANAGE_IMX6()
	-----------------
*/
void hid_manage_imx6(HANDLE hDevice)
{
uint32_t start_address;
imx_image_ivt fake;
uint32_t InputReportByteLength, OutputReportByteLength;

#ifdef _MSC_VER
	struct _HIDP_PREPARSED_DATA *preparsed;
	HIDP_CAPS capabilities;

	HidD_GetPreparsedData(hDevice, &preparsed);
	HidP_GetCaps(preparsed, &capabilities);
	InputReportByteLength = capabilities.InputReportByteLength;
	OutputReportByteLength = capabilities.OutputReportByteLength;

	HidD_FlushQueue(hDevice);

	HidD_FreePreparsedData(preparsed);
#elif defined(__APPLE__)
	CFNumberRef number;
	if ((number = (CFNumberRef)IOHIDDeviceGetProperty(hDevice, CFSTR(kIOHIDMaxInputReportSizeKey))) != NULL)
		CFNumberGetValue((CFNumberRef)number, kCFNumberSInt32Type, &InputReportByteLength);
	if ((number = (CFNumberRef)IOHIDDeviceGetProperty(hDevice, CFSTR(kIOHIDMaxOutputReportSizeKey))) != NULL)
		CFNumberGetValue((CFNumberRef)number, kCFNumberSInt32Type, &OutputReportByteLength);

	if (verbose)
		{
		printf("InputReportByteLength :%d\n", InputReportByteLength);
		printf("OutputReportByteLength:%d\n", OutputReportByteLength);
		}
#endif

/*
	Create the recieve and transmit buffers.  Unfortunately we don't know how large this is at
	compile time because its information returned by the HID device.  We, consequently
	must create it and pass that data around (so we make it global).

	transmit_double_buffer is needed for loading and storing the last "block" of RAM to avoid trashing
	the contents of the i.MX6Q RAM (due to a bug in its ROM)
*/
recieve_buffer = (unsigned char *)malloc(recieve_buffer_length = InputReportByteLength);
transmit_buffer = (unsigned char *)malloc(transmit_buffer_length = OutputReportByteLength);
transmit_double_buffer = (unsigned char *)malloc(OutputReportByteLength);

/*
	Here we expect the Usage Page to be 0xFF00 (Vendor-defined, see "Universal Serial Bus (USB) HID Usage
	Tables 6/27/2001 Version 1.11" page 16) and the usage to be 0x01 (i.e. its device type 0x01), but this
	is mostly not useful.
*/

if (elf_file != NULL)
	{
	/*
		We might be and ELF file or we might be an imximage file.
	*/
	if (elf_file[0] == 0x7F && elf_file[1] == 'E' && elf_file[2] == 'L' && elf_file[3] == 'F')
		{
		/*
			We're an ELF file to decode it into memory
		*/
		if (verbose)
			printf("Load ELF file '%s'\n", elf_filename);
		start_address = elf_load(hDevice, elf_file, (uint32_t)elf_file_length);

		/*
			We now need to "fake" an imximage ivt entry point.  The problem is to find somewhere
			in memory to put it.  We'll assume the few bytes before the start of the program
			are free and we'll use those.
		*/
		memset(&fake, 0, sizeof(fake));
		fake.header.tag = IMX_IMAGE_TAG_FILE_HEADER;
		fake.header.length = IMX_IMAGE_FILE_HEADER_LENGTH;
		fake.header.version = IMX_IMAGE_VERSION;
		fake.entry = start_address;
		fake.self = start_address - sizeof(fake);
		start_address = fake.self;
		if (verbose)
			printf("Send fake header to address: 0x%08X (it says start at:0%08X)\n",start_address, fake.entry);
		imx_write_file(hDevice, start_address, sizeof(fake), (unsigned char *)&fake);

		/*
			Finally we can branch to the stat of the imximage ivt header we just faked
		*/
		if (verbose)
			printf("Jump to 0x%08llX\n", (long long)start_address);
		imx_jump_address(hDevice, start_address);
		if (verbose)
			puts("Running");
		return;
		}
	else
		{
		/*
			We're and imximage file
			The file format of an imximage file is described in section 8.6 (starting on page 440) of "i.MX 6Dual/6Quad
			Applications Processor Reference Manual Rev. 0, 11/2012"
		*/
		if (verbose)
			printf("Load IMX file '%s'\n", elf_filename);
		start_address = imximage_load(hDevice, elf_file, (uint32_t)elf_file_length);
		if (verbose)
			printf("Jump to 0x%08llX\n", (long long)start_address);
		imx_jump_address(hDevice, start_address);
		if (verbose)
			puts("Running");
		return;
		}
	}
else
	go_be_a_monitor(hDevice);
}

#ifdef _MSC_VER
	/*
		HID_PRINT_GUID()
		----------------
		Print a Windows GUID to stdout
	*/
	void HID_print_guid(GUID *guid)
	{
	printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	}
#endif

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
long parameter, found = false;

verbose = false;
for (parameter = 1; parameter < argc; parameter++)
	{
	if (strcmp(argv[parameter], "-?") == 0)
		{
		help();
		return 0;
		}
	else if (strcmp(argv[parameter], "-verbose") == 0)
		verbose = true;
	else
		if ((elf_file = read_entire_file(elf_filename = argv[parameter], &elf_file_length)) == NULL)
			exit(printf("Can't open file:%s\n", elf_filename));

	}

#ifdef _MSC_VER
	GUID guid;
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	long MemberIndex;
	DWORD Required;
	SP_DEVICE_INTERFACE_DETAIL_DATA *detailData;
	HANDLE hDevice;
	HIDD_ATTRIBUTES Attributes;

	/*
		Get the GUID of the Windows HID class so that we can identify HID devices
	*/
	HidD_GetHidGuid(&guid);

	if (verbose)
		{
		printf("The USB GUID is:");
		HID_print_guid(&guid);
		puts("");
		}

	/*
		Get the list of HID devices
	*/
	if ((hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)) == INVALID_HANDLE_VALUE)
		puts("SetupDiGetClassDevs() failue");

	/*
		Iterate through the list of HID devices
	*/
	devInfoData.cbSize = sizeof(devInfoData);
	for (MemberIndex = 0; SetupDiEnumDeviceInterfaces(hDevInfo, 0, &guid, MemberIndex, &devInfoData); MemberIndex++)
		{
		Required = 0;
		/*
			Call SetupDiGetDeviceInterfaceDetail() to get the length of the device name then call it again to get the name itself
		*/
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &Required, NULL);

		/*
			Allocate space for the name of the HID device
		*/
		detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Required);
		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);	// should be 5

		/*
			Get the name and details of the HID device
		*/
		if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, Required, NULL, NULL))
			{
			if ((hDevice = CreateFile(detailData->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
				{
				char buffer[1024];
				COMMTIMEOUTS timeout = {1000, 0, 1000, 0, 1000};		// reads and writes timeout in one second
				if (verbose)
					printf("Found USB HID Device: %s\n", detailData->DevicePath);

				SetCommTimeouts(hDevice, &timeout);
				Attributes.Size = sizeof(Attributes);
				HidD_GetAttributes(hDevice, &Attributes);
				if (verbose)
					{
					printf("      VID:%04llx PID:%04llx Version:%lld (", (long long)Attributes.VendorID, (long long)Attributes.ProductID, (long long)Attributes.VersionNumber);
					HidD_GetManufacturerString(hDevice, buffer, sizeof(buffer));
					wprintf(L"%s", buffer);
					HidD_GetProductString(hDevice, buffer, sizeof(buffer));
					wprintf(L"%s)", buffer);
					puts("");
					}
				if (Attributes.VendorID == IMX_VID && Attributes.ProductID == IMX_PID)
					{
					found = true;
					if (verbose)
						puts("i.MX6Q device identified");
					hid_manage_imx6(hDevice);
					}
				CloseHandle(hDevice);
				}
			}
		/*
			Clean up the memory allocated to get the HID device name
		*/
		free(detailData);
		}

	/*
		Clean up the memory allocated when we got the device list
	*/
	SetupDiDestroyDeviceInfoList(hDevInfo);
#elif defined(__APPLE__)
	IOHIDManagerRef hid_manager;
	CFIndex number_of_devices;
	CFSetRef device_set;
	const IOHIDDeviceRef *device_array;
	IOHIDDeviceRef device;

	/*
		Get a handle to the HID manager
	*/
	hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

	/*
		Schedule the HID Manager into a MacOS X Main Run Loop.
	*/
	IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	/*
		Open the manager
	*/
	IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);

	/*
		Create a set of matching criteria
	*/
	CFDictionaryRef matchingCFDictRef = create_dictionary(IMX_PID, IMX_VID);

	/*
		Enumerate all HID devices that match the criteria and make sure we have only 1.
	*/
	IOHIDManagerSetDeviceMatching(hid_manager, matchingCFDictRef);

	device_set = IOHIDManagerCopyDevices(hid_manager);
	if (device_set == NULL)
		exit(printf("Cannot find any attached devices\n"));

	if ((number_of_devices = CFSetGetCount(device_set)) != 1)
		exit(printf("Device is not unique\n"));

	if (verbose)
		puts("i.MX6Q device identified");

	/*
		Get the list into a C++ array
	*/
	device_array = new IOHIDDeviceRef [number_of_devices];
	CFSetGetValues(device_set, (const void **)device_array);
	CFRelease(device_set);

	/*
		Get some stats about the device
	*/
	device = (IOHIDDeviceRef)*device_array;

	if (IOHIDDeviceOpen(device, kIOHIDOptionsTypeSeizeDevice) == kIOReturnSuccess)
		{
		found = true;
		hid_init(device);
		hid_manage_imx6(device);
		IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
		}
	else
		printf("Cannot open HID device\n");
	/*
		We're finished with the device set and device list so free them.
	*/
	delete [] device_array;
#endif

if (!found)
	puts("Cannot find an i.MX6Q attached");

return 0;
}
