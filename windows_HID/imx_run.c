/*
	IMX_RUN.C
	----------
	Copyright (c) 2013 Andrew Trotman, University of Otago
	Open Source under the BSD License.
*/
#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <SetupAPI.h>
#include <stdio.h>
#include <stdlib.h>

int true = (1 == 1);
int false = (1 == 0);

/*
	Standard types
*/
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef long int32_t;
typedef unsigned long long uint64_t;

#define IMX_VID 5538		// 0x15A2:Freescale
#define IMX_PID 84			// 0x0054 i.MX6Q


/*
	IMX_SERIAL_MESSAGE
	------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct _imx_serial_message
{
uint8_t report_id;
uint16_t command_type;
uint32_t address;			// in MSB-LSB format
uint8_t format;
uint32_t data_count;		// in MSB-LSB format
uint32_t data;
uint8_t reserved;
} imx_serial_message;
#pragma pack()

/*
	IMX_SERIAL_MESSAGE command types
	--------------------------------
*/
#define READ_REGISTER		0x0101
#define WRITE_REGISTER		0x0202
#define WRITE_FILE			0x0404
#define ERROR_STATUS		   0x0505
#define DCD_WRITE			   0x0A0A
#define JUMP_ADDRESS		   0x0B0B

/*
	IMX_SERIAL_MESSAGE formats
	--------------------------
*/
#define BITS_8  0x08
#define BITS_16 0x10
#define BITS_32 0x20

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
	ELF32_EHDR
	----------
	ELF file header
*/
typedef struct _Elf32_Ehdr
{
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
} Elf32_Ehdr;

/*
	ELF32_PHDR
	----------
	ELF program header
*/
typedef struct _Elf32_Phdr
{
Elf32_Word	p_type;
Elf32_Off	p_offset;
Elf32_Addr	p_vaddr;
Elf32_Addr	p_paddr;
Elf32_Size	p_filesz;
Elf32_Size	p_memsz;
Elf32_Word	p_flags;
Elf32_Size	p_align;
} Elf32_Phdr;

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
   the its length for any file we're going to upload (or NULL on none)
   and length of the file we'
*/
char *elf_file = NULL;
char *elf_filename = NULL;
long long elf_file_length = 0;

/*
	Verbose mode is a bit more explicit about the communications thats going on
*/
long verbose;

/*
	In Windows there are two ways to communicate with a HID device, one is using HidD_SetOutputReport() and HidD_GetInputReport()
	while the other is to use WriteFile() and ReadFile().  It turns out they are not equivelant.  the HidD methods send the
	messages to the control endpoint (endpoint 0) whereas the File methods use the interrupt endpoints.  It also appears
	as though when you call HidD_GetInputReport() the ID of the report you want is transferred to the device as part of the
	request.

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

printf("Secirity Descriptor Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);

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
message.data = 0;

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
	writing into long consequitive blocks of RAM.  So, we may as well make this method transmit only one
	byte.  Call it 4 times to write a 32-bit integer.

	This method returns 1 on success or 0 on failure
*/
uint32_t imx_write_register(HANDLE hDevice, uint32_t address, uint32_t byte)
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
message.format = BITS_8;
message.data_count = 1;
message.data = intel_to_arm(byte);

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
message.format = BITS_8;
message.data_count = intel_to_arm(bytes);
message.data = 0;

/*
	Due to a bug in the i.MX6Q ROM we need to read the last kilobyte from the i.MX6Q into the transmit
	buffer, then write our final packet over it and send that packet back.
*/
imx_read_register(hDevice, address + (bytes - (bytes % transmit_buffer_length)), (transmit_buffer_length - 1), transmit_double_buffer);

/*
	Transmit to the board and get the response back
*/
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
		if (!WriteFile(hDevice, transmit_buffer, transmit_buffer_length, &dwBytes, NULL))
			return 0;

		remaining -= current_buffer_length;
		from += current_buffer_length;
		}

	/*
		Get descriptors back
	*/
	if (imx_get_security_descriptor(hDevice))												// recieve Report 3 (security descriptor)
		if (ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))		// recieve Report 4
			if (memcmp(recieve_buffer, success, 5) == 0)									// check the status code
				return 1;
			else
				printf("Load File Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);
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
	informaton on the format of a DCD file
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
*/
uint32_t imx_jump_address(HANDLE hDevice, uint32_t address)
{
imx_serial_message message;
DWORD dwBytes = 0;

/*
	Set up the message block (Report 1)
*/
memset(&message, 0, sizeof(message));
message.report_id = 1;
message.command_type = JUMP_ADDRESS;
message.address = intel_to_arm(address);
message.format = BITS_8;
message.data_count = 0;
message.data = 0;

/*
	Transmit to the board and get the response back
*/
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))								// transmit Report 1
	if (imx_get_security_descriptor(hDevice))												      // recieve Report 3 (security descriptor)
		{
      /*
         At present this isn't returning (because the board is running) so we ignore the possible Report 4
      */
#ifdef NEVER
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

	returns the HAB status code (or 0 on failure)
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
message.address = 0;
message.format = BITS_8;
message.data_count = 0;
message.data = 0;

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
	We don't do any checking here because we already have control over the board pre-boot. and so
	it really doesn't matter what we download into it.

	Return the program entry point on success or -1 (0xFFFFFFFF) on error.
*/
uint32_t elf_load(HANDLE hDevice, const uint8_t *file, uint32_t length)
{
Elf32_Ehdr *header;			   // the ELF file header
uint32_t header_offset;		   // location (in the file) of the first header
uint32_t header_num;			   // number of headers
Elf32_Phdr *current_header;	// the current header
uint32_t which;				   // which header we're currenty looking at
char *buffer;                 // Used to create long string of zeros to clear the various sections of memory

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
   imx_write_file(hDevice, current_header->p_vaddr, current_header->p_memsz, buffer);
   imx_write_file(hDevice, current_header->p_vaddr, current_header->p_filesz, (char *)(file + current_header->p_offset));
   free(buffer);
	current_header++;
	}

return header->e_entry;
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
*/
char *strip_space_inplace(char *source)
{
char *end, *start = source;

while (isspace(*start))
	start++;

if (start > source)
	memmove(source, start, strlen(start) + 1);      // copy the '\0'

end = source + strlen(source) - 1;
while ((end >= source) && (isspace(*end)))
	*end-- = '\0';

return source;
}

/*
	READ_FILE_64()
	--------------
*/
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
		return 0;

return 1;
}

/*
	READ_ENTIRE_FILE()
	------------------
*/
char *read_entire_file(char *filename, long long *file_length)
{
long long unused;
char *block = NULL;
HANDLE fp;
LARGE_INTEGER details;
char *true_filename = filename;									// else we're in ASCII land and so the filename is a c-string

if (filename == NULL)
	return NULL;

if (file_length == NULL)
	file_length = &unused;

*file_length = 0;

if ((fp = CreateFile(true_filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
	return NULL;

if (GetFileSizeEx(fp, &details) != 0)
	if ((*file_length = details.QuadPart) != 0)
		if ((block = (char *)malloc((size_t)(details.QuadPart + 1))) != NULL)		// +1 for the '\0' on the end
			if (read_file_64(fp, block, details.QuadPart) != 0)
				block[details.QuadPart] = '\0';
			else
				{
				free(block);
				block = NULL;
				}

CloseHandle(fp);

return block;
}

/*
	GO_BE_A_MONITOR()
	-----------------
*/
void go_be_a_monitor(HANDLE hDevice)
{
unsigned char command[1024];
static unsigned char into[1024 * 1024];
uint64_t address = 0x00000000, bytes_to_read = 0x0F, byte, bytes_to_send = 0, write_address;
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
		printf("%08lX\n", status);
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
			imx_write_file(hDevice, (uint32_t)address, (uint32_t)bytes_to_send, data);
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
		imx_dcd_write(hDevice, (uint32_t)address, (uint32_t)bytes_to_send, data);
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
struct _HIDP_PREPARSED_DATA  *preparsed;
HIDP_CAPS capabilities;
uint32_t elf_start_address;

HidD_GetPreparsedData(hDevice, &preparsed);
HidP_GetCaps(preparsed, &capabilities);

/*
	Create the recieve and transmit buffers.  Unfortunately we don't know how large this is at
	compile time because its information returned by the HID device.  We, consequently
	must create it and pass that data around (so we make it global).

	transmit_double_buffer is needed for loading and storing the last "block" of RAM to avoid trashing
	the contents of the i.MX6Q RAM (due to a bug in its ROM)
*/
recieve_buffer = (unsigned char *)malloc(recieve_buffer_length = capabilities.InputReportByteLength);
transmit_buffer = (unsigned char *)malloc(transmit_buffer_length = capabilities.OutputReportByteLength);
transmit_double_buffer = (unsigned char *)malloc(capabilities.OutputReportByteLength);

/*
	Flush the internal buffers
*/
HidD_FlushQueue(hDevice);

/*
	Here we expect the Usage Page to be 0xFF00 (Vendor-defined, see "Universal Serial Bus (USB) HID Usage Tables 6/27/2001 Version 1.11" page 16)
	and the usage to be 0x01 (i.e. its device type 0x01), but this is mostly meaningless.
*/

if (elf_file != NULL)
   {
   if (verbose)
      printf("Load ELF file '%s'\n", elf_filename);
   elf_start_address = elf_load(hDevice, elf_file, (uint32_t)elf_file_length);
   if (verbose)
      printf("Jump to %08llX\n", (long long) elf_start_address);
   imx_jump_address(hDevice, elf_start_address);
   if (verbose)
      puts("Running");
   return;
   }
else
   go_be_a_monitor(hDevice);

HidD_FreePreparsedData(preparsed);
}

/*
	HID_PRINT_GUID()
	----------------
	Print a Windows GUID to stdout
*/
void HID_print_guid(GUID *guid)
{
printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
{
GUID guid;
HDEVINFO hDevInfo;
SP_DEVICE_INTERFACE_DATA devInfoData;
long MemberIndex;
DWORD Required;
SP_DEVICE_INTERFACE_DETAIL_DATA *detailData;
HANDLE hDevice;
HIDD_ATTRIBUTES Attributes;
long parameter, found = false;

verbose = false;
for (parameter = 0; parameter < argc; parameter++)
	{
	if (strcmp(argv[parameter], "-?") == 0)
		{
		help();
		return 0;
		}
	else if (strcmp(argv[parameter], "-verbose") == 0)
		verbose = true;
   else
      elf_file = read_entire_file(elf_filename = argv[parameter], &elf_file_length);
	}

/*
	get the GUID of the Windows HID class so that we can identify HID devices
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

if (!found)
	puts("Cannot find an i.MX6Q attached");

return 0;
}
