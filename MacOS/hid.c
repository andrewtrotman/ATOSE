/*
	HID.C
	-----
	List all the HID devices on Mac OS X
*/
#include <IOKit/hid/IOHIDLib.h>

/*
	MAIN()
	------
*/
int main(void)
{
IOHIDManagerRef hid_manager;
CFMutableArrayRef devArray;
char string_buffer[1024];
CFIndex number_of_devices;
CFSetRef device_set;
const IOHIDDeviceRef **device_array;
const IOHIDDeviceRef **current;
CFNumberRef vendor, product;
long vendor_id, product_id;
CFStringRef manufacturer, product_name;

/*
	Get a handle to the HID manager
*/
hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

/*
	Enumerate all HID devices and count how many we have.
*/
IOHIDManagerSetDeviceMatching(hid_manager, NULL);
IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
device_set = IOHIDManagerCopyDevices(hid_manager);
number_of_devices = CFSetGetCount(device_set);

/*
	Tell the user how many we found
*/
printf("%lld HID devices found\n", (long long)number_of_devices);

/*
	Get the list into a C++ array
*/
device_array = new const IOHIDDeviceRef *[number_of_devices];
CFSetGetValues (device_set, (const void **)device_array);

/*
	Iterate the device list
*/
for (current = device_array; current < device_array + number_of_devices; current++)
	{
	IOHIDDeviceRef hidDev = (IOHIDDeviceRef)*current;

	vendor_id = product_id = 0;

	if ((vendor = (CFNumberRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDVendorIDKey))) != NULL)
		CFNumberGetValue(vendor, kCFNumberSInt32Type, &vendor_id);
	printf("VID:%04lX ", vendor_id);

	if ((product = (CFNumberRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDProductIDKey))) != NULL)
		CFNumberGetValue((CFNumberRef)product, kCFNumberSInt32Type, &product_id);
	printf("PID:%04lX ", product_id);

	if ((manufacturer = (CFStringRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDManufacturerKey)))!= NULL)
		{
		CFStringGetCString(manufacturer, string_buffer, sizeof(string_buffer), kCFStringEncodingUTF8);
		printf("%s ", string_buffer);
		}

	if ((product_name = (CFStringRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDProductKey))) != NULL)
		{
		CFStringGetCString(product_name, string_buffer, sizeof(string_buffer), kCFStringEncodingUTF8);
		printf("(%s)", string_buffer);
		}

	puts("");
	}

/*
	We're finished with the device set and device list so free them.
*/
CFRelease(device_set);
delete [] device_array;

return 0;
}
