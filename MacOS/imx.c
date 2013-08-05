/*
	IMX.C
	-----
	List the HID destriptors from an i.MX6Q connected to a Mac running Mac OS X

	Copyright (c) 2013 Andrew Trotman, University of Otago
	Open Source under the BSD License.
*/
#include <IOKit/hid/IOHIDLib.h>

#define IMX6Q_PID 0x0054
#define IMX6Q_VID 0x15A2

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

/*
	GET_UINT32_FROM_DEVICE()
	------------------------
*/
uint32_t get_uint32_from_device(IOHIDDeviceRef device, CFStringRef parameter)
{
uint32_t result;
CFNumberRef number;

result = 0;
if ((number = (CFNumberRef)IOHIDDeviceGetProperty(device, parameter)) != NULL)
	CFNumberGetValue((CFNumberRef)number, kCFNumberSInt32Type, &result);

return result;
}

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


uint8_t recieve_buffer[2048];
size_t recieve_buffer_length = sizeof(recieve_buffer);

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

typedef uint32_t DWORD;

/*
	CALLBACK()
	----------
*/
void callback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength)
{
printf("Got a report of ID:%u\n", reportID);
*(long *)context = reportLength;
}

template <class T> T min(T a, T b) { return a < b ? a : b; }

/*
	READFILE()
	----------
*/
long ReadFile(IOHIDDeviceRef hDevice, void *recieve_buffer, uint32_t to_read, uint32_t *did_read, void *ignore)
{
volatile long done;

done = 0;
IOHIDDeviceRegisterInputReportCallback(hDevice, (uint8_t *)recieve_buffer, to_read, callback, (void *)&done);

do
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, true);
while (done == 0);

IOHIDDeviceRegisterInputReportCallback(hDevice, NULL, 0, NULL, NULL);

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
	IMX_GET_SECURITY_DESCRIPTOR()
	-----------------------------
	Read a Report 3 from the i.MX6 and make sure the decice is open. The report 3 is
	5 bytes long and will be either 0x0312343412 on a "closed" system or 0x0356787856
	on an open system.  We consider a closed system as failure,

	return 0 on failure or closed system, 1 on success (open)
*/
uint32_t imx_get_security_descriptor(IOHIDDeviceRef hDevice)
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
uint32_t imx_read_register(IOHIDDeviceRef hDevice, uint32_t address, uint32_t bytes, unsigned char *into)
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
puts("Send output report");
if (HidD_SetOutputReport(hDevice, &message, sizeof(message)))
	{
	puts("get security descriptor");
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
		puts("READ");
		while (remaining > 0)
			{
			puts("READ part");
			if (!ReadFile(hDevice, recieve_buffer, recieve_buffer_length, &dwBytes, NULL))
				return 0;

			dwBytes -= 1;	// skip over the report ID
			memcpy(into, recieve_buffer + 1, (size_t)(dwBytes > remaining ? remaining : dwBytes));

			into += dwBytes > remaining ? remaining : dwBytes;
			remaining -= dwBytes;
			}
		puts("DONE");
		}
	return bytes;		// success
	}

return 0;
}

/*
	MAIN()
	------
*/
int main(void)
{
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
CFDictionaryRef matchingCFDictRef = create_dictionary(IMX6Q_PID, IMX6Q_VID);

/*
	Enumerate all HID devices that match the criteria and make sure we have only 1.
*/
IOHIDManagerSetDeviceMatching(hid_manager, matchingCFDictRef);

device_set = IOHIDManagerCopyDevices(hid_manager);
if (device_set == NULL)
	exit(printf("Cannot find any attached devices\n"));

if ((number_of_devices = CFSetGetCount(device_set)) != 1)
	exit(printf("Device is not unique\n"));

puts("Found!");

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
	uint8_t into[1024];

	printf("Device OPEN\n");

	printf("kIOHIDMaxInputReportSizeKey:%d bytes\n", get_uint32_from_device(device, CFSTR(kIOHIDMaxInputReportSizeKey)));
	printf("kIOHIDMaxOutputReportSizeKey:%d bytes\n", get_uint32_from_device(device, CFSTR(kIOHIDMaxOutputReportSizeKey)));

	imx_read_register(device, 0, 4, into);
	printf("0000:%d %d %d %d\n", into[0], into[1], into[2], into[3]);
	IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
	}
else
	printf("Cannot open HID device\n");
/*
	We're finished with the device set and device list so free them.
*/
delete [] device_array;

return 0;
}
