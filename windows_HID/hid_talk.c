/*
	HID_TALK.C
	----------
	Copyright (c) 2013 Andrew Trotman, University of Otago
	Open Source under the BSD License.
*/
#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <SetupAPI.h>
#include <stdio.h>

/*
	Standard types
*/
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

#define IMX_VID 5538		// Freescale
#define IMX_PID 84		// i.MX6Q 

/*
	IMX_SERIAL_MESSAGE
	------------------
*/
#pragma pack(1)			// Microsoft's way of aligning on 1 byte boundaries
typedef struct
{
uint8_t ms_report_number;
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
#define ERROR_STATUS		0x0505
#define DCD_WRITE			0x0A0A
#define JUMP_ADDRESS		0x0B0B

/*
	IMX_SERIAL_MESSAGE formats
	--------------------------
*/
#define BITS_8  0x08
#define BITS_16 0x10
#define BITS_32 0x20

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
	IMX_READ_REGISTER()
	-------------------
	see Page 452 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
	I send a Report 1
	i.MX6Q sends a Report 3 (security)
	i.MX6Q sends a number of Report 4 (data in response)

	returns number of bytes read of 0 on failure
*/
long imx_read_register(HANDLE hDevice, uint32_t address, uint32_t bytes, unsigned char *into)
{
unsigned char buffer[65];
imx_serial_message message;
DWORD dwBytes = 0;
long long remaining;

/*
	Set up the message block
*/
memset(&message, 0, sizeof(message));
message.ms_report_number = 1;
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
		This can be checked as follows... the firt byte of buffer should be the report number (0x03)
		then the next 4 bytes will be either 0x12 0x34 0x34 0x12 in closed mode or 0x56 0x78 0x78 0x56
		in open mode.
	*/
	if (ReadFile(hDevice, buffer, sizeof(buffer), &dwBytes, NULL))
		{
		/*
			Recieve as many bytes as this method was asked to retrieve, but in 65-byte packets (on the i.MX6Q).
			We consequently need to joint all these together to build the answer
		*/
		remaining = bytes;
		while (remaining > 0)
			{
			if (!ReadFile(hDevice, buffer, sizeof(buffer), &dwBytes, NULL))
				return 0;

			dwBytes -= 1;	// skip over the report ID
			memcpy(into, buffer + 1, (size_t)(dwBytes > remaining ? remaining : dwBytes));

			into += dwBytes > remaining ? remaining : dwBytes;
			remaining -= dwBytes;
			}
		}
	return bytes;		// success
	}

return 0;
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
		printf("%c", isprint(buffer[column]) ? buffer[column] : ' ');

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
puts("COMMANDS");
puts("--------");
puts("help                   : display this message");
puts("quit                   : exit this program");
puts("read <address> <bytes> : read <bytes> from <address>.  Specify in Hex");
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
	GO_BE_A_MONITOR()
	-----------------
*/
void go_be_a_monitor(HANDLE hDevice)
{
unsigned char command[1024];
unsigned char into[1024];
uint64_t address, bytes_to_read;

do
	{
	printf("]");
	fgets(command, sizeof(command), stdin);
	strip_space_inplace(command);

	if (strcmp(command, "quit") == 0)
		break;
	else if (strcmp(command, "help") == 0)
		help();
	else if (strcmp(command, "?") == 0)
		help();
	else if (strncmp(command, "read ", 5) == 0)
		{
		sscanf(command + 5, "%llx %llx", &address, &bytes_to_read);
		if (bytes_to_read > sizeof(into))
			printf("Cannot dump more than %llX (dec:%lld) bytes in one read\n", (long long)sizeof(into), (long long)sizeof(into));
		else
			{
			imx_read_register(hDevice, (uint32_t)address, (uint32_t)bytes_to_read, into);
			dump_buffer(into, address, bytes_to_read);
			}
		}
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

HidD_GetPreparsedData(hDevice, &preparsed);
HidP_GetCaps(preparsed, &capabilities);

/*
	Flush the internal buffers
*/
HidD_FlushQueue(hDevice);

/*
	Here we expect the Usage Page to be 0xFF00 (Vendor-defined, see "Universal Serial Bus (USB) HID Usage Tables 6/27/2001 Version 1.11" page 16)
	and the usage to be 0x01 (i.e. its device type 0x01), but this is mostly meaningless.
*/

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
int main(void)
{
GUID guid;
HDEVINFO hDevInfo;
SP_DEVICE_INTERFACE_DATA devInfoData;
long MemberIndex;
DWORD Required;
SP_DEVICE_INTERFACE_DETAIL_DATA *detailData;
HANDLE hDevice;
HIDD_ATTRIBUTES Attributes;

/*
	get the GUID of the Windows HID class so that we can identify HID devices
*/
HidD_GetHidGuid(&guid);
#ifdef NEVER
	/*
		Print the GUID
	*/
	HID_print_guid(&guid);
	puts("");
#endif

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
//			printf("%s\n", detailData->DevicePath);

			Attributes.Size = sizeof(Attributes);
			HidD_GetAttributes(hDevice, &Attributes);
			printf("Found: VID:%04llx PID:%04llx Version:%lld (", (long long)Attributes.VendorID, (long long)Attributes.ProductID, (long long)Attributes.VersionNumber);
			HidD_GetManufacturerString(hDevice, buffer, sizeof(buffer));
			wprintf(L"%s", buffer);
			HidD_GetProductString(hDevice, buffer, sizeof(buffer));
			wprintf(L"%s)", buffer);
			puts("");
			if (Attributes.VendorID == IMX_VID && Attributes.ProductID == IMX_PID)
				hid_manage_imx6(hDevice);
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

return 0;
}
