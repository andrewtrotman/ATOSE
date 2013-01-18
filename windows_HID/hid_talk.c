#include <Windows.h>
#include "c:\WinDDK\7600.16385.1\inc\api\hidsdi.h"
#include <SetupAPI.h>

/*
	HID_PRINT_GUID()
	----------------
	Print a Windows GUID to stdout
*/
void HID_print_guid(GUID *guid)
{
printf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", guid->Data1, guid->Data2, guid->Data3, guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3], guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

int main(void)
{
GUID guid;
HDEVINFO hDevInfo;
SP_DEVICE_INTERFACE_DATA devInfoData;
long MemberIndex;
DWORD Required;
SP_DEVICE_INTERFACE_DETAIL_DATA *detailData;

/*
	get the GUID of the Windows HID class so that we can identify HID devices
*/
HidD_GetHidGuid(&guid);
HID_print_guid(&guid);
puts("");

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
		Get the name of the HID device
	*/
	if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, Required, NULL, NULL))
		{
		printf("%s\n", detailData->DevicePath);
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