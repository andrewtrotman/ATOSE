/*
	IMX.C
	-----
	List the HID destriptors from an i.MX6Q connected to a Mac running Mac OS X
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

/*
	IMX_GET_SECURITY_DESCRIPTOR()
	-----------------------------
	return 0 on failure or closed system, 1 on success (open)
*/
uint32_t imx_get_security_descriptor(IOHIDDeviceRef hDevice)
{
unsigned char open_system[] = {0x03, 0x56, 0x78, 0x78, 0x56};
CFIndex dwBytes = 0;
IOReturn success;

dwBytes = recieve_buffer_length;
success = IOHIDDeviceGetReport(hDevice, kIOHIDReportTypeInput, 0x03, recieve_buffer, &dwBytes);

if (success == kIOReturnSuccess)
	if (memcmp(recieve_buffer, open_system, 5) == 0)
		return 1;
	else
		printf("Secirity Descriptor Error:%02X%02X%02X%02X\n", recieve_buffer[1], recieve_buffer[2], recieve_buffer[3], recieve_buffer[4]);
else
	printf("Secirity Descriptor Error:Read failed\n");

return 0;
}

/*
	CALLBACK()
	----------
*/
void callback(void *context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength)
{
printf("Got a report of ID:%u\n", reportID);
}

/*
	IMX_READ_REGISTER()
	-------------------
	This method returns the number of bytes read or 0 on failure
*/
uint32_t imx_read_register(IOHIDDeviceRef hDevice, uint32_t address, uint32_t bytes, uint8_t *into)
{
imx_serial_message message;
CFIndex dwBytes = 0;

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
	Set up a Mac Callback for the reply
*/
dwBytes = recieve_buffer_length;


IOHIDDeviceRegisterInputReportCallback(hDevice, recieve_buffer, dwBytes, callback, NULL);
#ifdef NEVER
	if ((IOReturn tIOReturn = IOHIDDeviceGetReportWithCallback(hDevice, kIOHIDReportTypeInput, 0x03, recieve_buffer, recieve_buffer, 1, callback, NULL)) == kIOReturnSuccess)
		printf("successfully registered the callback\n");
	else
		printf( "\n%s: Calling IOHIDDeviceGetReportWithCallback FAILED tIOReturn = %lX.\n", __PRETTY_FUNCTION__, (unsigned long)tIOReturn);
#endif
/*
	Transmit to the board and get the response back
*/
if (IOHIDDeviceSetReport(hDevice, kIOHIDReportTypeOutput, 0x01, (uint8_t *)&message, sizeof(message)) == kIOReturnSuccess)
	{
	/*
		Start the Run Loop
	*/
	puts("Start Run Loop");
	int32_t got = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, true);
	printf("Run Loop Finished with:%d\n", got);
#ifdef NEVER
	if (imx_get_security_descriptor(hDevice))
		printf("Success!\n");
	else
		printf("Failed to get security descriptor\n");
#endif
	}
else
	printf("Transmit failed\n");

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
