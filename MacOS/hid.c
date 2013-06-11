/*
	HID.C
	-----
	List all the HID devices on Mac OS X
*/
#include <IOKit/hid/IOHIDLib.h>

IOHIDManagerRef hidManager;
CFMutableArrayRef devArray;

/*
	CFSETCOPYCALLBACK()
	-------------------
*/
void CFSetCopyCallBack(const void *value, void *context)
{
CFArrayAppendValue((CFMutableArrayRef)context, value);
}

/*
	IOHIDDEVICE_GETLONGPROPERTY()
	-----------------------------
*/
Boolean IOHIDDevice_GetLongProperty(IOHIDDeviceRef inIOHIDDeviceRef, CFStringRef inKey, long *outValue)
{
Boolean result = FALSE;
 
if (inIOHIDDeviceRef)
	{
	assert(IOHIDDeviceGetTypeID() == CFGetTypeID(inIOHIDDeviceRef));
 
	CFTypeRef tCFTypeRef = IOHIDDeviceGetProperty(inIOHIDDeviceRef, inKey);
 
	if (tCFTypeRef)
		if (CFNumberGetTypeID() == CFGetTypeID( tCFTypeRef))
			result = CFNumberGetValue((CFNumberRef) tCFTypeRef, kCFNumberSInt32Type, outValue);
	}
	
return result;
}


/*
	MAIN()
	------
*/
int main(void)
{
char string_buffer[1024];

/*
	Get a handle to the HID manager
*/
hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

/*
	Enumerate all HID devices.
*/
IOHIDManagerSetDeviceMatching(hidManager, NULL);
IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);

/*
	Get the device list and copy it.
*/
CFSetRef copyOfDevices = IOHIDManagerCopyDevices(hidManager);
devArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
CFSetApplyFunction(copyOfDevices, CFSetCopyCallBack, (void *)devArray);

/*
	Find out how many devices we've got.
*/
CFIndex nDev = CFArrayGetCount(devArray);
printf("%d devices found\n",(int)nDev);

/*
	Finished with the device list so free it.
*/
CFRelease(copyOfDevices);

/*
	Iterate the device list
*/
for (long device = 0; device < nDev; device++)
	{
	IOHIDDeviceRef hidDev = (IOHIDDeviceRef)CFArrayGetValueAtIndex(devArray, device);
	if (hidDev != NULL)
		{
		long vendor_id, product_id;

		vendor_id = product_id = 0;
		IOHIDDevice_GetLongProperty(hidDev, CFSTR(kIOHIDVendorIDKey), &vendor_id);
		IOHIDDevice_GetLongProperty(hidDev, CFSTR(kIOHIDProductIDKey), &product_id);
		printf("VID:%04lX PID:%04lX ", vendor_id, product_id);


		CFStringRef manufacturer = (CFStringRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDManufacturerKey));
		if (manufacturer != NULL)
			{
			CFStringGetCString(manufacturer, string_buffer, sizeof(string_buffer), kCFStringEncodingUTF8);
			printf("%s ", string_buffer);
			}

		CFStringRef product = (CFStringRef)IOHIDDeviceGetProperty(hidDev, CFSTR(kIOHIDProductKey));
		if (product != NULL)
			{
			CFStringGetCString(product, string_buffer, sizeof(string_buffer), kCFStringEncodingUTF8);
			printf("(%s)", string_buffer);
			}

		puts("");
		}
	}
return 0;
}