/*
	IMX233_USB.C
	-------------
	Experiments with the I.MX233 USB controller
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../source/registers.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/hw_irq.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsrtc.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsicoll.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspinctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsgpmi.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsclkctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsdigctl.h"

#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsapbx.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsusbctrl.h"
#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsusbphy.h"


#define USB_DIRECTION_OUT						0x00
#define USB_DIRECTION_IN						0x01

#define USB_REQ_GET_STATUS						0x00
#define USB_REQ_CLEAR_FEATURE					0x01
#define USB_REQ_SET_FEATURE					0x03
#define USB_REQ_SET_ADDRESS					0x05
#define USB_REQ_GET_DESCRIPTOR					0x06
#define USB_REQ_SET_DESCRIPTOR					0x07
#define USB_REQ_GET_CONFIGURATION				0x08
#define USB_REQ_SET_CONFIGURATION				0x09
#define USB_REQ_GET_FEATURE_DESCRIPTOR			0x0C

#define USB_CDC_SET_LINE_CODING				0x20
#define USB_CDC_GET_LINE_CODING				0x21
#define USB_CDC_SET_CONTROL_LINE_STATE			0x22

#define USB_REQ_MS_GET_EXTENDED_PROPERTIES		0x05

#define USB_DESCRIPTOR_TYPE_DEVICE				0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION		0x02
#define USB_DESCRIPTOR_TYPE_STRING				0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE			0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT			0x05
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER	0x06
#define USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIG	0x07
#define USB_DESCRIPTOR_TYPE_INTERFACE_POWER	0x08
#define USB_DESCRIPTOR_TYPE_OTG				0x09
#define USB_DESCRIPTOR_TYPE_CS_INTERFACE		0x24

#define USB_DESCRIPTOR_SUBTYPE_HEADER			0x00
#define USB_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT 0x01
#define USB_DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT 0x02
#define USB_DESCRIPTOR_SUBTYPE_UNION_FUNCTION	0x06

#define USB_CAPABILITIES_NO_CALL_MANAGEMENT	0x00 
#define USB_CAPABILITIES_CALL_MANAGEMENT_COMS	0x01
#define USB_CAPABILITIES_CALL_MANAGEMENT_DATA	0x03

#define USB_CAPABILITIES_CONNECTION  			0x08
#define USB_CAPABILITIES_BREAK       			0x04
#define USB_CAPABILITIES_LINE       			0x02
#define USB_CAPABILITIES_FEATURE      			0x01

#define USB_DEVICE_STATE_DEFAULT				0x00
#define USB_DEVICE_STATE_ADDRESSED				0x01
#define USB_DEVICE_STATE_CONFIGURED			0x02

#define USB_DESCRIPTOR_TYPE_CONFIGURATION_SELFPOWERED 0xC0
#define USB_DESCRIPTOR_TYPE_CONFIGURATION_REMOTEWAKE  0xA0

#define USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC 0x02
#define USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_ABSTRACT_CONTROL 0x02
#define USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_PROTOCOL_NONE 0x00
#define USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_PROTOCOL_HAYES 0x01
#define USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_DATA_INTERFACE 0x0A

//Constants for the endpoints we're going to define in our program:
#define DEVICE_ENDPOINT_CONTROL 						0			// The first endpoint is always a control endpoint
#define DEVICE_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT	1			// Endpoint 1 is the abstract control management interface
#define DEVICE_ENDPOINT_SERIAL_OUT						2			// Endpoint 2 will be serial out
#define DEVICE_ENDPOINT_SERIAL_IN	 					2			// Endpoint 2 will be serial in


#define USB_ENDPOINT_DIRECTION_IN						0x80
#define USB_ENDPOINT_DIRECTION_OUT						0x00

#define USB_ENDPOINT_TYPE_CONTROL						0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS					0x01
#define USB_ENDPOINT_TYPE_BULK							0x02
#define USB_ENDPOINT_TYPE_INTERRUPT					0x03

#define REG_SZ 1


void usb_setup_endpoint_nonzero(void);
void usb_setup_endpoint_zero(void);

/*
	struct USB_SETUP_PACKET
	-----------------------
	Setup packets are stored in the queue head, this structure makes
	it possible to access the seperate members by name
*/
struct usb_setup_packet
{
uint8_t bmRequestType;
uint8_t bRequest;
uint16_t wValue;
uint16_t wIndex;
uint16_t wLength;
} __attribute__ ((packed));

/*
	struct ENDPOINT_TD
	------------------
	Endpoint Transfer Descriptor data struct
	must be 32-bye alligned (and is 32-bytes long).
*/
struct endpoint_td
{
struct endpoint_td *next_td_ptr;		/* Next TD pointer(31-5), T(0) set indicate invalid */
uint32_t size_ioc_sts;					/* Total bytes (30-16), IOC (15), MultO(11-10), STS (7-0) */
void *buff_ptr0;						/* Buffer pointer Page 0 (31-12) */
void *buff_ptr1;						/* Buffer pointer Page 1 (31-12) */
void *buff_ptr2;						/* Buffer pointer Page 2 (31-12) */
void *buff_ptr3;						/* Buffer pointer Page 3 (31-12) */
void *buff_ptr4;						/* Buffer pointer Page 4 (31-12) */
uint32_t reserved1;					/* Reserved */
} __attribute__ ((packed));

/*
	struct ENDPOINT_QUEUE_HEAD
	--------------------------
*/
struct endpoint_queue_head
{
uint32_t max_pkt_length;				/* bitfield: Mult(31-30) , Zlt(29) , Max Pkt len  and IOS(15) */
void *curr_dtd_ptr;					/* Current dTD Pointer(31-5). This is set by the USB controller */
struct endpoint_td td_overlay_area; 	/* dTD Overlay Area - I believe this is a 32 byte copy of the current dTD written by the USB controller, except for the "next" field which we should set to our first dTD address when we prime the endpoint. */
struct usb_setup_packet setup_buffer;	/* setup packets are dumped here on arrival */
uint32_t reserved2[4];					/* Needed to guarantee 64-byte allignment */
} __attribute__ ((packed));

/*
	struct USB_DEVICE_DESCRIPTOR
	----------------------------
	This is the device descripter that we send back to the host
*/
struct usb_device_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t bcdUSB;
uint8_t bDeviceClass;
uint8_t bDeviceSubClass;
uint8_t bDeviceProtocol;
uint8_t bMaxPacketSize0;
uint16_t idVendor;
uint16_t idProduct;
uint16_t bcdDevice;
uint8_t iManufacturer;
uint8_t iProduct;
uint8_t iSerialNumber;
uint8_t bNumConfigurations;
} __attribute__ ((packed));

/*
	OUR_DEVICE_DESCRIPTOR
	---------------------
	We're going top fake being a serial port
*/
struct usb_device_descriptor our_device_descriptor =
{
.bLength = sizeof(our_device_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
.bcdUSB = 0x0200, 				// USB 2.0
.bDeviceClass = 0x02, 			// Communications Device Class (Pretend to be a Serial Port)
.bDeviceSubClass = 0x00,
.bDeviceProtocol = 0x00,
.bMaxPacketSize0 = 64, 		// Accept 64 bytes packet size on control endpoint (the maximum)
.idVendor = 0xDEAD,			// Manufacturer's ID
.idProduct = 0x000E,			// Product's ID
.bcdDevice = 0x0100,			// Product Verison Number (revision number)
.iManufacturer = 0x01,			// string ID of the manufacturer
.iProduct = 0x02,				// string ID of the Product
.iSerialNumber = 0x03,			// string ID of the serial number
.bNumConfigurations = 0x01
};



/*
	struct USB_DEVICE_QUALIFIER
	---------------------------
	This gets used when a high-speed device is plugged in so that the host
	can determine what would happen if plugged into a non-high-speed port!
*/
struct usb_device_qualifier
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t bcdUSB;
uint8_t bDeviceClass;
uint8_t bDeviceSubClass;
uint8_t bDeviceProtocol;
uint8_t bMaxPacketSize0;
uint8_t bNumConfigurations;
uint8_t reserved;
} __attribute__ ((packed));

/*
	OUR_DEVICE_QUALIFIER
	--------------------
*/
struct usb_device_qualifier our_device_qualifier =
{
.bLength = sizeof(our_device_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
.bcdUSB = 0x0200, 				// USB 2.0
.bDeviceClass = 0x02, 			// Communications Device Class (Pretend to be a Serial Port)
.bDeviceSubClass = 0x00,
.bDeviceProtocol = 0x00,
.bMaxPacketSize0 = 64, 		// Accept 64 bytes packet size on control endpoint (the maximum)
.bNumConfigurations = 0x01,
.reserved = 0x00
} ;

/*
	struct USB_LANGUAGE
	-------------------
*/
struct usb_language
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t wLANGID;
}  __attribute__ ((packed));

/*
	OUR_LANGUAGE
	------------
*/
struct usb_language our_language =
{
.bLength = sizeof(our_language),
.bDescriptorType = 0x03,
.wLANGID = 0x1409
};

/*
	struct USB_STRING
	-----------------
*/
struct usb_string
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t wString[22];
}  __attribute__ ((packed));

/*
	OUR_SERIAL_NUMBER
	-----------------
*/
struct usb_string our_serial_number =
{
.bLength = 14,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'S', 0x00, 'N', 0x00, '#', 0x00, 'D', 0x00, 'E', 0x00, 'V', 0x00}
} ;

/*
	OUR_MANUFACTURER
	----------------
*/
struct usb_string our_manufacturer =
{
.bLength = 10,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'A', 0x00, 'S', 0x00, 'P', 0x00, 'T', 0x00}
} ;

/*
	OUR_PRODUCT
	-----------
*/
struct usb_string our_product =
{
.bLength = 16,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'F', 0x00, 'o', 0x00, 'u', 0x00, 'r', 0x00, 'A', 0x00, 'R', 0x00, 'M', 0x00}
} ;

/*
	struct USB_CONFIG_DESCRIPTOR
	----------------------------
*/
struct usb_config_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t wTotalLength;
uint8_t bNumInterfaces;
uint8_t bConfigurationValue;
uint8_t iConfiguration;
uint8_t bmAttributes;
uint8_t bMaxPower;
} __attribute__ ((packed));

/*
	struct USB_ENDPOINT_DESCRIPTOR
	------------------------------
*/
struct usb_endpoint_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bEndpointAddress;
uint8_t bmAttributes;
uint16_t wMaxPacketSize;
uint8_t bInterval;
} __attribute__ ((packed));

/*
	struct USB_INTERFACE_DESCRIPTOR
	-------------------------------
*/
struct usb_interface_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bInterfaceNumber;
uint8_t bAlternateSetting;
uint8_t bNumEndpoints;
uint8_t bInterfaceClass;
uint8_t bInterfaceSubClass;
uint8_t bInterfaceProtocol;
uint8_t iInterface;
} __attribute__ ((packed));

/*
	struct USB_CDC_FUNCTIONAL_DESCRIPTOR
	------------------------------------
*/
struct usb_cdc_functional_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint16_t bcdUSB;
} __attribute__ ((packed));

/*
	struct USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	----------------------------------------------------
*/
struct usb_cdc_call_management_functional_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint8_t bmCapabilities;
uint8_t bDataInterface;
} __attribute__ ((packed));

/*
	struct USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	----------------------------------------------------------------
*/
struct usb_cdc_abstract_control_management_functional_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint8_t bmCapabilities;
} __attribute__ ((packed));

/*
	struct USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR
	------------------------------------------------
*/
struct usb_union_interface_functional_descriptor
{
uint8_t bFunctionLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubtype;
uint8_t bMasterInterface;
uint8_t bSlaveInterface0;
} __attribute__ ((packed));



/*
	stuct USB_COM_DESCRIPTOR
	------------------------
	For a COM port we have three end points
*/

struct usb_com_descriptor
{
struct usb_config_descriptor cd;

struct usb_interface_descriptor id0;
		struct usb_cdc_functional_descriptor fd1;
		struct usb_cdc_call_management_functional_descriptor fd2;
		struct usb_cdc_abstract_control_management_functional_descriptor fd3;
		struct usb_union_interface_functional_descriptor fd4;
			struct usb_endpoint_descriptor ep2;

	struct usb_interface_descriptor id1;
		struct usb_endpoint_descriptor ep3;
		struct usb_endpoint_descriptor ep4;
} __attribute__ ((packed));

/*
	I based this on the USB Serial Example for Teensy USB Development
	Board (http://www.pjrc.com/teensy/usb_serial.html) but as it isn't a
	"substantial portions of the Software" and that software is under a
	BSD-like licence, I don't reproduce their copyright notice here.

	The version of the CDC spec being referenced is "Universal Serial Bus
	Class Definitions for Communication Devices Version 1.1 January 19,
	1999"

	The version of the USB spec being referenced is "Universal Serial Bus
	Specification Revision 2.0 April 27, 2000"

	Interface 0:
		Abstract Control Management
			Endpoint 2 (OUT from host) Interrupt
	Interface 1:
		Data
			Endpoint 3 (OUT from host) Bulk
			Endpoint 4 (IN to host)  Bulk
*/
struct usb_com_descriptor our_com_descriptor =
{
	{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	sizeof(struct usb_config_descriptor),										// bLength;
	USB_DESCRIPTOR_TYPE_CONFIGURATION,											// bDescriptorType;
	sizeof(struct usb_com_descriptor),											// wTotalLength
	2,																			// bNumInterfaces
	1,																			// bConfigurationValue (id)
	0,																			// iConfiguration (string descriptor)
	USB_DESCRIPTOR_TYPE_CONFIGURATION_SELFPOWERED,								// bmAttributes
	50																			// bMaxPower
	},
	{
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	sizeof(struct usb_interface_descriptor),									// bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,												// bDescriptorType
	0,																			// bInterfaceNumber
	0,																			// bAlternateSetting
	1,																			// bNumEndpoints
	USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC,									// bInterfaceClass
	USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_ABSTRACT_CONTROL,					// bInterfaceSubClass
	USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_PROTOCOL_HAYES,						// bInterfaceProtocol (Hayes Model AT command set)
	0																			// iInterface (string descriptor)
	},
	{
	// CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
	sizeof(struct usb_cdc_functional_descriptor),								// bFunctionLength
	USB_DESCRIPTOR_TYPE_CS_INTERFACE,											// bDescriptorType
	USB_DESCRIPTOR_SUBTYPE_HEADER,												// bDescriptorSubtype
	0x0110																		// bcdCDC (version of the CDC spec we adhere to (in this case v1.1))
	},
	{
	// Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
	sizeof(struct usb_cdc_call_management_functional_descriptor),				// bFunctionLength
	USB_DESCRIPTOR_TYPE_CS_INTERFACE,											// bDescriptorType
	USB_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT,										// bDescriptorSubtype	(call management)
	USB_CAPABILITIES_CALL_MANAGEMENT_COMS,										// bmCapabilities (call management over the communications class interface)
	1																			// bDataInterface (use this class for call management)
	},
	{
	// Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
	sizeof(struct usb_cdc_abstract_control_management_functional_descriptor),	// bFunctionLength
	USB_DESCRIPTOR_TYPE_CS_INTERFACE,											// bDescriptorType
	USB_DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT,							// bDescriptorSubtype
	USB_CAPABILITIES_BREAK | USB_CAPABILITIES_LINE								// bmCapabilities (handle line and break management)
	},
	{
	// Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
	sizeof(struct usb_union_interface_functional_descriptor),					// bFunctionLength
	USB_DESCRIPTOR_TYPE_CS_INTERFACE,											// bDescriptorType
	USB_DESCRIPTOR_SUBTYPE_UNION_FUNCTION,										// bDescriptorSubtype
	0,																			// bMasterInterface (The interface number of the Communication or Data Class interface, designated as the master or controlling interface for the union)
	1																			// bSlaveInterface0 (Interface number of first slave or associated interface in the union.)
	},
	{
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	sizeof(struct usb_endpoint_descriptor),										// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,												// bDescriptorType
	DEVICE_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT | USB_ENDPOINT_DIRECTION_IN,	// bEndpointAddress (Endpoint number | direction (in | out) IN going to host)
	USB_ENDPOINT_TYPE_INTERRUPT,												// bmAttributes (Interrupt end point)
	16,																			// wMaxPacketSize
	64																			// bInterval (polling interval)
	},
	{
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	sizeof(struct usb_interface_descriptor),									// bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,												// bDescriptorType
	1,																			// bInterfaceNumber
	0,																			// bAlternateSetting
	2,																			// bNumEndpoints
	USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_DATA_INTERFACE,							// bInterfaceClass
	0x00,																		// bInterfaceSubClass
	USB_DESCRIPTOR_TYPE_INTERFACE_CLASS_CDC_PROTOCOL_NONE,						// bInterfaceProtocol
	0																			// iInterface (string desceiptor)
	},
	{
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	sizeof(struct usb_endpoint_descriptor),										// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,												// bDescriptorType
	DEVICE_ENDPOINT_SERIAL_OUT | USB_ENDPOINT_DIRECTION_OUT,					// bEndpointAddress (OUTGOING from host)
	USB_ENDPOINT_TYPE_BULK,														// bmAttributes (0x02=bulk)
	64,																			// wMaxPacketSize
	0																			// bInterval
	},
	{
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	sizeof(struct usb_endpoint_descriptor),										// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,												// bDescriptorType
	DEVICE_ENDPOINT_SERIAL_IN | USB_ENDPOINT_DIRECTION_IN,						// bEndpointAddress (INGOING to host)
	USB_ENDPOINT_TYPE_BULK,														// bmAttributes (0x02=bulk)
	64,																			// wMaxPacketSize
	0																			// bInterval
	}
};

/*
	USB CDC stuff
	=============
*/

/*
	struct USB_CDC_LINE_CODING
	--------------------------
*/
struct usb_cdc_line_coding
{
uint32_t dwDTERat;		// Number Data terminal rate, in bits per second.
uint8_t bCharFormat;	// Number Stop bits (0 = 1 Stop bit, 1 = 1.5 Stop bits, 2 = 2 Stop bits)
uint8_t bParityType;	// Number Parity (0 = None, 1 = Odd, 2 = Even, 3 = Mark, 4 = Space)
uint8_t bDataBits;		// Number Data bits (5, 6, 7, 8 or 16).
} ;

/*
	OUR_CDC_LINE_CODING
	-------------------
	9600,n,8,1
*/
struct usb_cdc_line_coding our_cdc_line_coding =
{
.dwDTERat = 115200,
.bCharFormat = 0,
.bParityType =  0,
.bDataBits =  8
} ;


/*
	Microsoft descriptor extensions
	===============================
*/
/*
	struct USB_MS_OS_STRING_DESCRIPTOR
	----------------------------------
*/
struct usb_ms_os_string_descriptor
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t qwSignature[14];
uint8_t bMS_VendorCode;
uint8_t bPad;
} ;

/*
	OUR_MS_OS_STRING_DESCRIPTOR
	---------------------------
*/
struct usb_ms_os_string_descriptor our_ms_os_string_descriptor = 
{
.bLength = sizeof(our_ms_os_string_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.qwSignature = {'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, '1', 0x00, '0', 0x00, '0', 0x00},     //{0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00},	//MSFT100
.bMS_VendorCode = 0,
.bPad = 0
};


/*
	struct USB_MS_COMPATIBLE_ID_FEATURE_DESCRIPTOR_HEADER
	-----------------------------------------------------
*/
struct usb_ms_compatible_id_feature_descriptor_header
{
uint32_t dwLength;
uint16_t bcdVersion;
uint16_t wIndex;
uint8_t bCount;
uint8_t reserved1[7];
};

/*
	struct USB_MS_COMPATIBLE_ID_FEATURE_FUNCTION_SECTION
	----------------------------------------------------
*/
struct usb_ms_compatible_id_feature_function_section
{
uint8_t bFirstInterfaceNumber;
uint8_t reserved1;
uint8_t compatibleID[8];
uint8_t subCompatibleID[8];
uint8_t reserved2[6];
} ;

/*
	struct USB_MS_COMPATIBLE_ID_FEATURE_DESCRIPTOR
	----------------------------------------------
*/
struct usb_ms_compatible_id_feature_descriptor
{
struct usb_ms_compatible_id_feature_descriptor_header header;
struct usb_ms_compatible_id_feature_function_section section1;
} ;

/*
	OUR_MS_COMPATIBLE_ID_FEATURE_DESCRIPTOR
	---------------------------------------
*/
struct usb_ms_compatible_id_feature_descriptor our_ms_compatible_id_feature_descriptor =
{
	{
	.dwLength = sizeof(our_ms_compatible_id_feature_descriptor),
	.bcdVersion = 1,
	.wIndex = 4,
	.bCount = 1,
	.reserved1 = {0},
	},
	{
	.bFirstInterfaceNumber = 0,
	.reserved1 = 0,
	.compatibleID = {0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00},
	.subCompatibleID = {0},
	.reserved2 = {0}
	}
} ;


/*
	struct USB_MS_EXTENDED_PROPERTY_HEADER
	--------------------------------------
*/
struct usb_ms_extended_property_header
{
uint32_t dwLength;				// sizeof whole descriptor
uint16_t bcdVersion;			// verion 0x0100
uint16_t wIndex;				// USB_REQ_MS_GET_EXTENDED_PROPERTIES
uint16_t wCount;
} ;

/*
	struct USB_MS_CUSTOM_PROPERTY_LABEL
	-----------------------------------
*/
struct usb_ms_custom_property_label
{
uint32_t dwSize;
uint32_t dwPropertyDataType;
uint16_t wPropertyNameLength;
uint8_t bPropertyName[12];				// "Label" in Unicode
uint32_t dwPropertyDataLength;
uint8_t bPropertyData[16];				// "FourARM" in Unicode
} ;


/*
	struct USB_MS_EXTENDED_PROPERTIES
	---------------------------------
*/
struct usb_ms_extended_properties
{
struct usb_ms_extended_property_header header;
struct usb_ms_custom_property_label property1;
} ;

/*
	OUR_MS_EXTENDED_PROPERTIES
	--------------------------
*/
struct usb_ms_extended_properties our_ms_extended_properties =
{
	{
	sizeof(our_ms_extended_properties),
	0x0100,
	USB_REQ_MS_GET_EXTENDED_PROPERTIES,
	1
	},
	{
	sizeof(struct usb_ms_custom_property_label),
	REG_SZ,
	12,
	{'L', 0x00, 'a', 0x00, 'b', 0x00, 'e', 0x00, 'l', 0x00, 0x00, 0x00},
	16,
	{'F', 0x00, 'o', 0x00, 'u', 0x00, 'r', 0x00, 'A', 0x00, 'R', 0x00, 'M', 0x00, 0x00, 0x00}
	}
} ;


/*
	Now on to the i.MX233
	=====================
*/

/*
	Pointers to memory (use for allocators)
*/
char *external_memory_head = (char *) 0x40000000;
char *internal_memory_head;

/*
	Queue heads must lie in internal memory for controller speed requirements.
	Lower 11 bits of address cannot be set so we must have a 2K alignment.

	Queuehead 0: OUT (USB Control from host)
	Queuehead 1: IN  (USB Control to host)
	Queuehead 2: OUT (Abstract Control Management from host)
	Queuehead 3: IN (Abstract Control Management to host) 
	Queuehead 4: OUT (Data from host) 
	Queuehead 5: IN  (Data to host)
*/
#define ENDPOINT_CONTROL_OUT	USB_DIRECTION_OUT
#define ENDPOINT_CONTROL_IN 	USB_DIRECTION_IN
#define ENDPOINT_ACM_OUT		2 // not used
#define ENDPOINT_ACM_IN		3
#define ENDPOINT_DATA_OUT		4
#define ENDPOINT_DATA_IN		5

struct endpoint_queue_head endpoint_queueheads[10] __attribute__ ((aligned (2048)));
struct endpoint_td global_dTD[10] __attribute__ ((aligned (32)));
uint8_t *global_transfer_buffer[10];			// should really be 4K aligned

int usb_state = USB_DEVICE_STATE_DEFAULT;

/*
	Interrupt Service Routines
*/
void __cs3_isr_undef(void) {}
void __cs3_isr_pabort(void) {}
void __cs3_isr_dabort(void) {}
void __cs3_isr_reserved(void) {}
void __cs3_isr_fiq(void) {}
void ATOSE_isr_swi(void) {}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char ch)
{
int loop = 0;

while (HW_UARTDBGFR_RD() & BM_UARTDBGFR_TXFF)
	if (++loop > 10000)
		break;

/* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
HW_UARTDBGDR_WR(ch);
}

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex(int data)
{
int i = 0;
char c;

for (i = sizeof(int) * 2 - 1; i >= 0; i--) 
	{
	c = data >> (i * 4);
	c &= 0xf;
	if (c > 9)
		debug_putc(c - 10 + 'A');
	else
		debug_putc(c + '0');
	}
}

/*
	DEBUG_PRINT_HEX()
	-----------------
*/
void debug_print_hex_byte(uint8_t data)
{
char *string = "0123456789ABCDEF";

debug_putc(string[(data >> 4) & 0x0F]);
debug_putc(string[data & 0x0F]);
}

/*
	DEBUG_PRINT_STRING()
	--------------------
*/
void debug_print_string(const char *string)
{
while (*string != 0)
	debug_putc(*string++);
}

/*
	DEBUG_PRINT_BITS()
	------------------
*/
void debug_print_bits(uint32_t i)
{
uint32_t bit;

for (bit = 31;; bit--) 
	{
	if (i & (1u << bit))
		debug_putc('1');
	else
		debug_putc('0');

	if (bit == 0)
		break;

	if (bit % 4 == 0)
		debug_putc(' ');
	}
}

/*
	DEBUG_PRINT_THIS()
	------------------
*/
void debug_print_this(char *start, uint32_t hex, char *end)
{
debug_print_string(start);
debug_print_hex(hex);
debug_print_string(end);
debug_print_string("\r\n");
}

/*
	DEBUG_PRINT_THIS_BITS()
	-----------------------
*/
void debug_print_this_bits(char *start, uint32_t hex, char *end)
{
debug_print_string(start);
debug_print_bits(hex);
debug_print_string(end);
debug_print_string("\r\n");
}

/*
	DEBUG_PRINT_BUFFER()
	--------------------
*/
void debug_print_buffer(char * buffer, int length, int abs_addr)
{
int i, j;
int colsize = 32;

debug_print_string("\r\n");

for (i = 0; i < length; i++)
	{
	//Print leading address at beginning of line
	if (i % colsize == 0)
		{
		if (abs_addr)
			debug_print_hex((uint32_t) buffer + i);
		else
			debug_print_hex(i);
		debug_putc(' ');
		}

	debug_print_hex_byte(buffer[i]);

	/*
	* Print blanks up til the end of the line if we're finishing up.
	*/
	if (i == length - 1 && length % colsize != 0)
		{
		for (j = 0; j < colsize - length % colsize; j++)
			debug_print_string("  ");
		}

	//Print ASCII representation at end of line
	if (i % colsize == colsize - 1 || i == length - 1)
		{
		debug_print_string("  ");

		for (j = 0; j < colsize; j++)
			{
			char c = buffer[i + j - colsize + 1];

			if (c >= 32 && c < 127)
				debug_putc(c);
			else
				debug_putc('?');
			}

		debug_print_string("\r\n");
		}
	}
}

/*
	MAIN_MEMORY_ALLOC()
	-------------------
*/
void* main_memory_alloc(size_t size, unsigned int alignment)
{
//Align the beginning to the requested alignment
uint32_t realignment = alignment - ((uint32_t) external_memory_head % alignment);

if (realignment > 0 && realignment < alignment)
	external_memory_head += realignment;

void *result = external_memory_head;
external_memory_head += size;

return result;
}

/*
	INTERNAL_MEMORY_ALLOC()
	-----------------------
*/
void* internal_memory_alloc(size_t size, int alignment)
{
//Align the beginning to the requested alignment
uint32_t realignment = alignment - ((uint32_t) internal_memory_head % alignment);

if (realignment > 0 && realignment < alignment)
	internal_memory_head += realignment;

void * result = internal_memory_head;

internal_memory_head += size;

if (internal_memory_head > (char*) 0x00007fff)
	{
	debug_print_string("Failed to allocate internal memory! Asked for ");
	debug_print_hex(size);
	debug_print_string(" bytes.\r\n");
	for (;;);		// hang
	}


debug_print_string("[ALLOC:");
debug_print_hex((uint32_t)result);
debug_print_string("]");

return result;
}

/*
	DELAY_US()
	----------
*/
void delay_us(unsigned long int us)
{
unsigned long int start = HW_DIGCTL_MICROSECONDS_RD();
unsigned long int end = start + us;

while ((int32_t) (HW_DIGCTL_MICROSECONDS_RD() - end) < 0)
	;/* nothing */
}

/*
	USB_PRIME_ENDPOINT()
	--------------------
	Queue up a transfer descriptor for the given endpoint
*/
void usb_prime_endpoint(int endpoint, int direction)
{
//Now get controller to activate these new TDs by priming the endpoints

uint32_t endpoint_prime_mask = 0;

switch (endpoint & 0x1)
	{
	case USB_DIRECTION_OUT:
//		debug_print_this("  Prime OUT on EP:", endpoint >> 1, "");
		endpoint_prime_mask = BF_USBCTRL_ENDPTPRIME_PERB(1 << (endpoint >> 1));
		break;
		break;
	case USB_DIRECTION_IN:
//		debug_print_this("  Prime IN on EP:", endpoint >> 1, "");
		endpoint_prime_mask = BF_USBCTRL_ENDPTPRIME_PETB(1 << (endpoint >> 1));
		break;
	default:
		debug_print_string("Bad direction");
	}

HW_USBCTRL_ENDPTPRIME_SET(endpoint_prime_mask);

while ((HW_USBCTRL_ENDPTPRIME_RD() & endpoint_prime_mask) != 0)
	; /* do nothing */
}

/*
	USB_QUEUE_TD_IN()
	-----------------
	IN towards the host
 * Queue up a transfer descriptor on the given endpoint which sends data inwards to the host.
 *
 * endpoint - Endpoint index
 * direction - One of USB_DIRECTION_OUT, USB_DIRECTION_IN
 * ioc - Interrupt on completion
 */
void usb_queue_td_in(int endpoint, const char *buffer, unsigned int length, int ioc)
{
struct endpoint_td *dTD = &global_dTD[endpoint];
memset(dTD, 0, sizeof(*dTD));

struct endpoint_queue_head *dQH = &endpoint_queueheads[endpoint];

dTD->buff_ptr0 = (char *)buffer;
dTD->next_td_ptr = (struct endpoint_td *) 0x01; //No TD follow this one
dTD->size_ioc_sts =
	length << 16
//	| (ioc ? 1 : 0) << 15
	| 1 << 15
	| 0x80; //Status = active

dQH->td_overlay_area.next_td_ptr = dTD; //Activate this TD by clearing the 'T' bit in the queue head next ptr

usb_prime_endpoint(endpoint, USB_DIRECTION_IN); //Signal the controller to start processing this queuehead
}

/*
	USB_QUEUE_TD_OUT()
	------------------
	OUT from the host
 * Queue up a transfer descriptor on the given endpoint which receives data sent out from the host.
 *
 * endpoint - Endpoint index
 * direction - One of USB_DIRECTION_OUT, USB_DIRECTION_IN
 * ioc - Interrupt on completion
*/
void usb_queue_td_out(int endpoint, unsigned int length, int ioc)
{
length = 64;		// this is the size of the rec buffer (the parameter is ignored)
struct endpoint_td *dTD =  &global_dTD[endpoint];
memset(dTD, 0, sizeof(*dTD));

struct endpoint_queue_head *dQH = &endpoint_queueheads[endpoint];

dTD->buff_ptr0 = (char *)global_transfer_buffer[endpoint];
dTD->next_td_ptr = (struct endpoint_td *) 0x01; //No TD follow this one
dTD->size_ioc_sts =
	length << 16
//	| (ioc ? 1 : 0) << 15
	| 1 << 15
	| 0x80; //Status = active

dQH->td_overlay_area.next_td_ptr = dTD; //Activate this TD by clearing the 'T' bit in the queue head next ptr

usb_prime_endpoint(endpoint, USB_DIRECTION_OUT); //Signal the controller to start processing this queuehead
}


/*
	PRINT_SETUP_PACKET()
	--------------------
*/
void print_setup_packet(struct usb_setup_packet req)
{
debug_print_string("SETUP PACKET\r\n");
debug_print_this("bmRequestType:", req.bmRequestType, "");
debug_print_this("bRequest:", req.bRequest, "");
debug_print_this("wValue:", req.wValue, "");
debug_print_this("wIndex:", req.wIndex, "");
debug_print_this("wLength:", req.wLength, "");
}

/*
	HANDLE_USB_GET_DESCRIPTOR()
	---------------------------
*/
void handle_usb_get_descriptor(struct usb_setup_packet req)
{
int descriptor_type = req.wValue >> 8;
int descriptor_index = req.wValue & 0xFF;
int descriptor_length = req.wLength;
uint32_t len;

switch (descriptor_type)
	{
	case USB_DESCRIPTOR_TYPE_DEVICE:
		debug_print_string("USB_DESCRIPTOR_TYPE_DEVICE\r\n");

		if (req.wLength != sizeof(our_device_descriptor))		// should probably be sizeof(our_device_descriptor), not 0x40
			len = 8;
		else
			len = sizeof(our_device_descriptor);

		usb_queue_td_in(ENDPOINT_CONTROL_IN, (char*)&our_device_descriptor, len, 0);

		//We expect to receive a zero-byte confirmation from the host, and we'll ask for an interrupt when that occurs
		usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);

		break;
	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_CONFIGURATION\r\n");

		usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_com_descriptor, sizeof(our_com_descriptor), 0);

		//We expect to receive a zero-byte confirmation from the host, and we'll ask for an interrupt when that occurs
		usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
		break;
	case USB_DESCRIPTOR_TYPE_STRING:
		switch (descriptor_index)
			{
			case 0xEE:
				debug_print_string("MS OS STRING DESCRIPTOR\r\n");
				/*
					Microsoft OS String Desciptor
				*/
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_ms_os_string_descriptor, sizeof(our_ms_os_string_descriptor), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				break;
			case 0x00:
				debug_print_string("STRING - LANG ID\r\n");
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_language, sizeof(our_language), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				break;
			case 0x01:
				debug_print_string("STRING - MANUFACTURER\r\n");
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_manufacturer, sizeof(our_manufacturer), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				break;
			case 0x02:
				debug_print_string("STRING - PRODUCT\r\n");
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_product, sizeof(our_product), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				break;
			case 0x03:
				debug_print_string("STRING - SERIAL NUMBER\r\n");
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_serial_number, sizeof(our_serial_number), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				break;
			default:
				debug_print_this("Unhandled STRING DESCRIPTOR:" , descriptor_index, "");
				break;
			}
		break;
	case USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
		debug_print_string("USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER\r\n");

		usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_device_qualifier, sizeof(our_device_qualifier), 0);

		//We expect to receive a zero-byte confirmation from the host, and we'll ask for an interrupt when that occurs
		usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
		break;
	default:
		print_setup_packet(req);
		debug_print_string("  this had lead to\r\n");
		debug_print_string("Unknown USB descriptor request received:\r\n");
		debug_print_this("Type:" , descriptor_type, "");
		debug_print_this("Index:" , descriptor_index, "");
		debug_print_this("Length:" , descriptor_length, "");
	}
}


/*
	HANDLE_USB_INTERRUPT()
	----------------------
*/
void handle_usb_interrupt(void)
{
int handled = 0;
uint16_t new_address;

//Do we have a pending endpoint setup event to handle?
if (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 0x1F)
	{
	if (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 1)
		{
//		debug_print_string("USB SETUP request.\r\n");
	
		struct usb_setup_packet setup_packet;

		/* We'll be slowly copying the setup packet from the endpoint descriptor.
		*
		* While we are doing that, another setup packet might arrive, and we'd
		* end up with a corrupt setup packet.
		*
		* So we set the tripwire and the hardware will clear it for us if another
		* packet arrives while we're busy:
		*/
		do
			{
			HW_USBCTRL_USBCMD.B.SUTW = 1;
			setup_packet = endpoint_queueheads[USB_DIRECTION_OUT].setup_buffer;
			}
		while (HW_USBCTRL_USBCMD.B.SUTW == 0); //Keep looping until we succeed

		HW_USBCTRL_USBCMD.B.SUTW = 0;

		// Clear the bit in the setup status register by writing a 1 there:
		do
			HW_USBCTRL_ENDPTSETUPSTAT_WR(1);
		while (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 1);

/*
			debug_print_string("SETUP PACKET\r\n");
			debug_print_this("bmRequestType:", setup_packet.bmRequestType, "");
			debug_print_this("bRequest:", setup_packet.bRequest, "");
			debug_print_this("wValue:", setup_packet.wValue, "");
			debug_print_this("wIndex:", setup_packet.wIndex, "");
			debug_print_this("wLength:", setup_packet.wLength, "");
*/

		if (setup_packet.bmRequestType & 0x80)
			{
			//Packets with a device-to-host data phase direction

			if (setup_packet.bmRequestType == 0xC0)		//USB_REQ_GET_FEATURE_DESCRIPTOR:
				{
				debug_print_string("USB_REQ_GET_MS_DESCRIPTOR\r\n");
				usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_ms_compatible_id_feature_descriptor, sizeof(our_ms_compatible_id_feature_descriptor), 0);
				usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
				}
			else if (setup_packet.bmRequestType == 0xC1)
				{
				switch (setup_packet.wIndex)
					{
					case USB_REQ_MS_GET_EXTENDED_PROPERTIES:
						debug_print_string("USB_REQ_MS_GET_EXTENDED_PROPERTIES\r\n");

						usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_ms_extended_properties, sizeof(our_ms_extended_properties), 0);
						usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
						break;
					default:
						print_setup_packet(setup_packet);
						debug_print_this("Unhandled GET_MS_DESCRIPTOR  ", setup_packet.wIndex, " was received.");
					}
				}
			else if (setup_packet.bmRequestType == 0xA1)		// CDC commands
				{
				switch (setup_packet.bRequest)
					{
					case USB_CDC_GET_LINE_CODING:
						debug_print_string("USB_CDC_GET_LINE_CODING\r\n");
						usb_queue_td_in(ENDPOINT_CONTROL_IN, (char *)&our_cdc_line_coding, sizeof(our_cdc_line_coding), 0);
						usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
						break;
					default:
						print_setup_packet(setup_packet);
						debug_print_this("Unhandled device-to-host data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			else
				{
				switch (setup_packet.bRequest)
					{
					case USB_REQ_GET_DESCRIPTOR:
						handle_usb_get_descriptor(setup_packet);
						handled = 1;
						break;
					default:
						print_setup_packet(setup_packet);
						debug_print_this("Unhandled device-to-host data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			}
		else
			{
			if (setup_packet.bmRequestType == 0x21)		// CDC commands (Abstract Control Model)	
				{
				switch (setup_packet.bRequest)
					{
					case USB_CDC_SET_CONTROL_LINE_STATE:								// this means the PC is online
						debug_print_string("USB_CDC_SET_CONTROL_LINE_STATE\r\n");
						usb_queue_td_in(ENDPOINT_CONTROL_IN, 0, 0, 0);				// respond with a 0-byte transfer (ACK)
						usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);
						break;
					case USB_CDC_SET_LINE_CODING:										// this tells us "9600,n,8,1" stuff (so we ignore)
//						print_setup_packet(setup_packet);
						debug_print_string("USB_CDC_SET_LINE_CODING\r\n");

						usb_queue_td_in(ENDPOINT_CONTROL_IN, 0, 0, 0);				// (ACK)
						usb_queue_td_out(ENDPOINT_CONTROL_OUT, 0, 1);

						/*
							Print the packet contents
						*/
						{
						int ch;
						uint8_t *into = (uint8_t *)&our_cdc_line_coding;
						for (ch = 0; ch < setup_packet.wLength; ch++)
							{
							debug_print_hex((global_transfer_buffer[ENDPOINT_CONTROL_OUT])[ch]);
							*into++ = (global_transfer_buffer[ENDPOINT_CONTROL_OUT])[ch];
							debug_putc(' ');
							}
						debug_print_string("\r\n");
						}
						break;
					default:
						print_setup_packet(setup_packet);
						debug_print_this("Unhandled device-to-host data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			else
				{
				switch (setup_packet.bRequest)
					{
					case USB_REQ_SET_CONFIGURATION:
						debug_print_string("USB_REQ_SET_CONFIGURATION\r\n");
						usb_queue_td_in(ENDPOINT_CONTROL_IN, 0, 0, 0);		// respond with a 0-byte transfer
						break;

					case USB_REQ_SET_ADDRESS:
						debug_print_string("USB_REQ_SET_ADDRESS\r\n");
						new_address = setup_packet.wValue;

						HW_USBCTRL_DEVICEADDR_WR(BF_USBCTRL_DEVICEADDR_USBADR(new_address) | BM_USBCTRL_DEVICEADDR_USBADRA);

						usb_queue_td_in(ENDPOINT_CONTROL_IN, 0, 0, 0);		// respond with a 0-byte transfer

						usb_state = USB_DEVICE_STATE_ADDRESSED;
						handled = 1;
						debug_print_this("                       Host has set our address to ", new_address, ".");
						usb_setup_endpoint_nonzero();
						break;
					default:
						//Packets with a host-to-device data phase direction
						print_setup_packet(setup_packet);
						debug_print_this("Unhandled host-to-device data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			}
	
		handled = 1;
		}
	else
		debug_print_this("SETUP PACKET arrived on endpoint other than 0 (HW_USBCTRL_ENDPTSETUPSTAT = ", HW_USBCTRL_ENDPTSETUPSTAT_RD(), ")");
	}
else
	{
	int managed = 0;
	int endpt;

	uint32_t endpoint_status = HW_USBCTRL_ENDPTSTAT_RD();
	uint32_t endpoint_setup_status = HW_USBCTRL_ENDPTSETUPSTAT_RD();
	uint32_t endpoint_complete = HW_USBCTRL_ENDPTCOMPLETE_RD(); 

	debug_print_this("  HW_USBCTRL_ENDPTSTAT      = ", endpoint_status, "");
	debug_print_this("  HW_USBCTRL_ENDPTSETUPSTAT = ", endpoint_setup_status, "");
	debug_print_this("  HW_USBCTRL_ENDPTCOMPLETE  = ", endpoint_complete, "");

	HW_USBCTRL_ENDPTSETUPSTAT_WR(endpoint_setup_status);
	HW_USBCTRL_ENDPTCOMPLETE_WR(endpoint_complete);

	for (endpt = 0; endpt < 5; endpt++)
		{
		if (endpoint_status & BF_USBCTRL_ENDPTSTAT_ERBR(1 << endpt))
			{
			debug_print_this("Recieve Endpoint Ready (", endpt, ")");
			if (endpt != 0)
				{
				debug_print_this("  Queue Status:", global_dTD[endpt * 2].size_ioc_sts, "");
				debug_print_this("  Listen on ", endpt * 2, "");
				usb_queue_td_out(endpt * 2, 1024, 1);
				}

			managed = 1;
			}
		if (endpoint_status & BF_USBCTRL_ENDPTSTAT_ETBR(1 << endpt))
			{
			debug_print_this("Transmit Endpoint Ready (", endpt, ")");
			managed = 1;
			}
		if (endpoint_complete & BF_USBCTRL_ENDPTCOMPLETE_ETCE(1 << endpt))
			{
			int byte;

			debug_print_this("Transmit Endpoint Complete (", endpt, ")");
			debug_print_string("   ");
			for (byte = 0; byte < 5; byte++)
				{
				debug_print_hex((global_transfer_buffer[endpt])[byte]);
				debug_print_string(" ");
				}
			debug_print_string("\r\n");

			managed = 1;
			}
		if (endpoint_complete & BF_USBCTRL_ENDPTCOMPLETE_ERCE(1 << endpt))
			{
			int byte;

			debug_print_this("Recieve Endpoint Complete (", endpt, ")");

			debug_print_string("   ");
			for (byte = 0; byte < 5; byte++)
				{
				debug_print_hex((global_transfer_buffer[endpt * 2])[byte]);
				debug_print_string(" ");
				}
			debug_print_string("\r\n");

			managed = 1;

			if (endpt != 0)
				{
				debug_print_this("  Listen on Queuehead: ", endpt * 2, "");
				usb_queue_td_out(endpt * 2, 1024, 1);
				usb_queue_td_in(endpt * 2 + 1, (char  *)global_transfer_buffer[endpt * 2], 1, 1);
				}
			}
		}

	if (managed)
		handled = 1;
	else
		{
		debug_print_string("Unknown interrupt source (HW_USBCTRL_ENDPTSTAT = ");
		debug_print_hex(endpoint_status);
		debug_print_string(") (HW_USBCTRL_ENDPTSETUPSTAT = ");
		debug_print_hex(endpoint_setup_status);
		debug_print_string(")\r\n");
		}
	}

if (!handled)
	debug_print_string("We got a USB interrupt, but we didn't do anything in response.\r\n");
}

/*
	ATOSE_ISR_IRQ()
	---------------
*/
uint32_t ATOSE_isr_irq(ATOSE_registers *registers)
{
volatile uint32_t got = 0;

/*
	Tell the ICOLL we've entered the ISR.  This is either a side-effect of the read or a write is required
*/
got = HW_ICOLL_VECTOR_RD();
got = HW_ICOLL_STAT_RD();


if (got == VECTOR_IRQ_USB_CTRL)
	{
	debug_print_string(" [USB] ");
	hw_usbctrl_usbsts_t usb_status = HW_USBCTRL_USBSTS;

	if (usb_status.B.TI0)
		{
		debug_print_string(" [I: USB TI0] \r\n");

		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_TI0);
		}
	else if (usb_status.B.TI1)
		{
		debug_print_string(" [I: USB TI1] \r\n");

		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_TI1);
		}
	else if (usb_status.B.URI)
		{
		debug_print_string(" [I: USB reset] \r\n");

		usb_state = USB_DEVICE_STATE_DEFAULT;

		//We clear our device address when we're reset:
		HW_USBCTRL_DEVICEADDR_WR(0);

		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_URI);
		}
	else if (usb_status.B.UEI)
		{
		int ep;
		debug_print_string(" [I: USB error interrupt ]");
		
		for (ep = 0; ep < 5; ep++)
			{
			debug_print_string("status ");
			debug_print_hex(ep);
			debug_print_string(" : ");
			debug_print_hex(global_dTD[ep].size_ioc_sts & 0xFF);
			debug_print_string(")\r\n");
			}


		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_UEI);
		}
	else if (usb_status.B.UI)
		{
		debug_print_string(" [I: USB UI Interrupt ] \r\n");
		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_UI);

		handle_usb_interrupt();
		} 
	else if (usb_status.B.PCI)
		{
		debug_print_string(" [I: USB Port Change Interrupt ] \r\n");
		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_PCI);
		} 
	else 
		{
		debug_print_string(" [I: Unknown USB interrupt ] \r\n");
		}
	} 
else 
	{
	debug_print_string("[->");
	debug_print_hex(got);
	debug_print_string("<-]\r\n");
	}

//TODO signal the APBX that we've handled an interrupt

/*
	Tell the interrupt controller that we've finished processing the Interrupt
*/
HW_ICOLL_LEVELACK_WR(BV_ICOLL_LEVELACK_IRQLEVELACK__LEVEL0);

return 0;
}

/*
	GET_CPSR()
	----------
*/
uint32_t get_cpsr(void)
{
uint32_t answer;

asm volatile ("mrs %0,CPSR" :"=r" (answer));
return answer;
}

/*
	SET_CPSR()
	----------
*/
void set_cpsr(uint32_t save_cpsr)
{
asm volatile ("msr CPSR_cxsf, %0"::"r"(save_cpsr));
}

/*
	ENABLE_IRQ()
	------------
*/
void enable_IRQ(void)
{
set_cpsr(get_cpsr() & ~0x80);
}

/*
	APBX_RESET()
	------------
*/
void apbx_reset()
{
//Clear SFTRST
HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_SFTRST);

//Wait for SFTRST to fall
do
	delay_us(1);
while (HW_APBX_CTRL0.B.SFTRST);

//Clear CLKGATE to wait for SFTRST to assert it
HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_CLKGATE);

//Soft reset
HW_APBX_CTRL0_SET(BM_APBX_CTRL0_SFTRST);

//Wait for CLKGATE to be brought high by the reset process
while (!HW_APBX_CTRL0.B.CLKGATE)
	; //Nothing

//Bring out of reset
HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_SFTRST);

//Wait for that to complete
do
	delay_us(1);
while (HW_APBX_CTRL0.B.SFTRST);

//Enable clock again
HW_APBX_CTRL0_CLR(BM_APBX_CTRL0_CLKGATE);

//Wait for that to complete
do
	delay_us(1);
while (HW_APBX_CTRL0.B.CLKGATE);
}

/*
	USB_PHY_STARTUP()
	-----------------
*/
void usb_phy_startup()
{
//	HW_CLKCTRL_XBUS.B.DIV = 20; // "make sure XBUS is lower than HBUS"
HW_CLKCTRL_XBUS.B.DIV = 1;

HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_EN_USB_CLKS);

//First do a soft-reset of the PHY

//Clear SFTRST
HW_USBPHY_CTRL_CLR(BM_USBPHY_CTRL_SFTRST);

//Wait for SFTRST to fall
do
	delay_us(1);
while (HW_USBPHY_CTRL.B.SFTRST);

//Clear CLKGATE to wait for SFTRST to assert it
HW_USBPHY_CTRL_CLR(BM_USBPHY_CTRL_CLKGATE);

//Soft reset
HW_USBPHY_CTRL_SET(BM_USBPHY_CTRL_SFTRST);

//Wait for CLKGATE to be brought high by the reset process
while (!HW_USBPHY_CTRL.B.CLKGATE)
	; //Nothing

//Bring out of reset
HW_USBPHY_CTRL_CLR(BM_USBPHY_CTRL_SFTRST);

//Wait for that to complete
do
	delay_us(1);
while (HW_USBPHY_CTRL.B.SFTRST);

//Enable clock again
HW_USBPHY_CTRL_CLR(BM_USBPHY_CTRL_CLKGATE);

//Wait for that to complete
do
	delay_us(1);
while (HW_USBPHY_CTRL.B.CLKGATE);

HW_USBPHY_PWD_WR(0); //Bring USB PHY components out of powerdown state

//Clear HW_POWER_CTRL_CLKGATE (we will assume this already happened in powerprep)

//For some unexplained reason we should:
HW_POWER_DEBUG_SET
	(
	BM_POWER_DEBUG_VBUSVALIDPIOLOCK |
	BM_POWER_DEBUG_AVALIDPIOLOCK |
	BM_POWER_DEBUG_BVALIDPIOLOCK
	);

HW_POWER_STS_SET
	(
	BM_POWER_STS_VBUSVALID |
	BM_POWER_STS_AVALID |
	BM_POWER_STS_BVALID
	);
}

/*
	CONFIG_ENDPOINT()
	-----------------
*/
void config_endpoint(int which)
{
struct endpoint_td *td;

//td = main_memory_alloc(sizeof(*td), 32);
td = &global_dTD[which];

memset(td, 0, sizeof(*td));
td->next_td_ptr = (struct endpoint_td *)1;
//td->size_ioc_sts = 1 << 15; //Interrupt on completion

endpoint_queueheads[which].td_overlay_area.next_td_ptr = (struct endpoint_td *)(((uint32_t)td) + 1);			// end of chain
endpoint_queueheads[which].td_overlay_area.size_ioc_sts = 1 << 15; 	//Interrupt on completion
global_transfer_buffer[which] = (uint8_t *)main_memory_alloc(1024, 1024);			// should really be 4K aligned though.
endpoint_queueheads[which].td_overlay_area.buff_ptr0 = (void *)global_transfer_buffer[which];

if (which == ENDPOINT_CONTROL_OUT || which== ENDPOINT_CONTROL_IN)
	endpoint_queueheads[which].max_pkt_length = (64 << 16) | (1 << 15); 	// 64 byte max packet size, interrupt on setup, ZLP
else
	endpoint_queueheads[which].max_pkt_length = 64 << 16; 	// 64 byte max packet size
}


/*
	USB_SETUP_ENDPOINT_ZERO()
	-------------------------
*/
void usb_setup_endpoint_zero()
{
/*
	Zero everything
*/
memset(endpoint_queueheads, 0, (sizeof *endpoint_queueheads));

/*
	Queue 0 and 1 : USB Control
*/
config_endpoint(ENDPOINT_CONTROL_OUT);
config_endpoint(ENDPOINT_CONTROL_IN);				// EP 0

/*
	Hand off to the i.MX233
*/
HW_USBCTRL_ENDPOINTLISTADDR_SET((uint32_t)endpoint_queueheads);
}

/*
	USB_SETUP_ENDPOINTS_NONZERO()
	-----------------------------
*/
void usb_setup_endpoint_nonzero()
{
hw_usbctrl_endptctrln_t endpointcfg;

/*
	Configure the i.MX233 structures
*/
/*
	Queue 0 and 1 : USB Control
*/
config_endpoint(ENDPOINT_CONTROL_OUT);
config_endpoint(ENDPOINT_CONTROL_IN);				// EP 0

/*
	Queue 2 and 3: Abstract Control Management Interface
*/
endpointcfg.B.TXE = 1; //TX endpoint enable
endpointcfg.B.TXR = 1; 	// reset the PID
endpointcfg.B.TXI = 0;
endpointcfg.B.TXT = BV_USBCTRL_ENDPTCTRLn_TXT__INT;
endpointcfg.B.TXD = 0;
endpointcfg.B.TXS = 0;

endpointcfg.B.RXE = 1; //RX endpoint enable
endpointcfg.B.RXR = 1;		// reset the PID
endpointcfg.B.RXI = 0;
endpointcfg.B.RXT = BV_USBCTRL_ENDPTCTRLn_RXT__INT;
endpointcfg.B.RXD = 0;
endpointcfg.B.RXS = 0;

config_endpoint(ENDPOINT_ACM_OUT);
config_endpoint(ENDPOINT_ACM_IN);
HW_USBCTRL_ENDPTCTRLn_WR(DEVICE_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT, endpointcfg.U);		// EP 1

/*
	Queue 4 and 5: Data from host
*/
endpointcfg.U = 0;
endpointcfg.B.TXE = 1; //TX endpoint enable
endpointcfg.B.TXR = 1; 	// reset the PID
endpointcfg.B.TXI = 0;
endpointcfg.B.TXT = BV_USBCTRL_ENDPTCTRLn_TXT__BULK;
endpointcfg.B.TXD = 0;
endpointcfg.B.TXS = 0;

endpointcfg.B.RXE = 1; //RX endpoint enable
endpointcfg.B.RXR = 1;		// reset the PID
endpointcfg.B.RXI = 0;
endpointcfg.B.RXT = BV_USBCTRL_ENDPTCTRLn_RXT__BULK;
endpointcfg.B.RXD = 0;
endpointcfg.B.RXS = 0;

config_endpoint(ENDPOINT_DATA_OUT);
config_endpoint(ENDPOINT_DATA_IN);
HW_USBCTRL_ENDPTCTRLn_WR(DEVICE_ENDPOINT_SERIAL_OUT, endpointcfg.U);		// EP 2


endpointcfg.U = 0;
HW_USBCTRL_ENDPTCTRLn_WR(3, endpointcfg.U);		// disable EP 3
endpointcfg.U = 0;
HW_USBCTRL_ENDPTCTRLn_WR(4, endpointcfg.U);		// disable EP 4

/*
	Hand off to the i.MX233
*/
HW_USBCTRL_ENDPOINTLISTADDR_SET((uint32_t)endpoint_queueheads);

/*
	Prime the endpoints
*/
usb_queue_td_out(ENDPOINT_ACM_OUT, 64, 1);
usb_queue_td_out(ENDPOINT_DATA_OUT, 64, 1);
}


/*
	USB_CONTROLLER_STARTUP()
	------------------------
*/
void usb_controller_startup()
{
//Turn on clocks to the USB block
HW_DIGCTL_CTRL_CLR(BM_DIGCTL_CTRL_USB_CLKGATE);

//USB shouldn't be running but let's just verify
if (HW_USBCTRL_USBCMD.B.RS)
	{
	//Stop USB
	HW_USBCTRL_USBCMD_CLR(BM_USBCTRL_USBCMD_RS);

	//We should be waiting for detach to complete here...
	delay_us(20000);
	}

//Reset USB
HW_USBCTRL_USBCMD_SET(BM_USBCTRL_USBCMD_RST);

while (HW_USBCTRL_USBCMD.B.RST)
	; //Wait for reset to complete

//Clear interrupt status bits that can apply to us as a Device

//These ones are actually cleared by setting them:
HW_USBCTRL_USBSTS_SET
	(
	BM_USBCTRL_USBSTS_URI | // USB Reset Enable 			(ASPT)
	BM_USBCTRL_USBSTS_PCI | // USB Port Change Detect Enable	(ASPT)
	BM_USBCTRL_USBSTS_TI0 |
	BM_USBCTRL_USBSTS_TI1 |
	BM_USBCTRL_USBSTS_SRI |
	BM_USBCTRL_USBSTS_URI
	);

//This one has unspecified clear behaviour so I'm going to assume I can just CLR
HW_USBCTRL_USBSTS_CLR(BM_USBCTRL_USBSTS_UI);

//We're a USB device not a USB host
HW_USBCTRL_USBMODE.B.CM = BV_USBCTRL_USBMODE_CM__DEVICE;

HW_USBCTRL_USBINTR_WR
	(
	//		BM_USBCTRL_USBINTR_SRE | //Interrupt me on Start-Of-Frame
	BM_USBCTRL_USBINTR_URE | // USB Reset Enable 			(ASPT)
	BM_USBCTRL_USBINTR_PCE | // USB Port Change Detect Enable	(ASPT)

	BM_USBCTRL_USBINTR_UE | //USBINT
	BM_USBCTRL_USBINTR_SEE | //System error
	BM_USBCTRL_USBINTR_TIE0 | //Interrupt on general purpose timer 0
	BM_USBCTRL_USBINTR_TIE1 //Interrupt on general purpose timer 1
	);

HW_USBCTRL_DEVICEADDR_WR(0); //This should be the reset default but let's make sure

HW_USBCTRL_ENDPTSETUPSTAT_WR(0);

usb_setup_endpoint_zero();

HW_USBCTRL_PORTSC1.B.PSPD = 0; //Full speed (12Mbps) (default)

//Disable setup lockout mode
HW_USBCTRL_USBMODE.B.SLOM = 1;

HW_USBCTRL_USBCMD.B.RS = 1; //Run
}

/*
	C_ENTRY()
	---------
*/
void c_entry(void)
{
uint32_t irq_stack[256];
uint32_t *irq_sp = irq_stack + sizeof(irq_stack);

/*
	Magic to get around the brownout problem in the FourARM
*/
HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;

/*
	call all C++ static object constructors
*/
extern void (*start_ctors)();
extern void (*end_ctors)();
void (**constructor)() = &start_ctors;

while (constructor < &end_ctors)
	{
	(*constructor)();
	constructor++;
	}

/*
	Find the top of memory
*/
extern uint32_t ATOSE_start_of_heap;

internal_memory_head = (char *) &ATOSE_start_of_heap;

//Set up the IRQ stack
asm volatile
	(
	"mov r2, %[stack];"
	"mrs r0, cpsr;"							// get the current mode
	"bic r1, r0, #0x1F;"					// turn off the mode bits
	"orr r1, r1, #0x12;"					// turn on the IRQ bits
	"msr cpsr, r1;"							// go into IRQ mode
	"mov sp, r2;"							// set the stack top
	"msr cpsr, r0;"							// back to original mode
	:
	: [stack]"r"(irq_sp)
	: "r0", "r1", "r2"
	);

/*
	Move the interrupt vector table to 0x00000000
*/
asm volatile
	(
	"MRC p15, 0, R0, c1, c0, 0;"			// read control register
	"AND R0, #~(1<<13);"					// turn off the high-interrupt vector table bit
	"MCR p15, 0, R0, c1, c0, 0;"			// write control register
	:
	:
	: "r0"
	);

/*
	Program Interrupt controller (i.MX233 ICOLL)
*/
HW_ICOLL_CTRL_WR(0); // reset the interrupt controller

/*
	Enable USB WAKE UP and USB CTRL
*/
HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_USB_WAKEUP, BM_ICOLL_INTERRUPTn_ENABLE);
HW_ICOLL_INTERRUPTn_SET(VECTOR_IRQ_USB_CTRL, BM_ICOLL_INTERRUPTn_ENABLE);

/*
	Tell the PIC to pass interrupts on to the CPU (and do ARM-style ISRs)
*/
HW_ICOLL_CTRL_SET(
	BM_ICOLL_CTRL_ARM_RSE_MODE |
	BM_ICOLL_CTRL_IRQ_FINAL_ENABLE
	);

/*
	Now we allow IRQ interrupts
*/
enable_IRQ();

/*
	Reset the APBX
*/
debug_print_string("\r\n\r\n-------------------\r\n\r\n");
debug_print_string("APBX reset...");
apbx_reset();
debug_print_string(" done!\r\n");

/*
	Enable the PHY (physical interface)
*/
debug_print_string("USB PHY startup... ");
usb_phy_startup();
debug_print_string(" done!\r\n");

/*
	Start the USB Controller
*/
debug_print_string("USB controller startup... ");
usb_controller_startup();
debug_print_string(" done!\r\n");

debug_print_string("\r\nSystem online.\r\n");

debug_print_string("USB status register: ");
debug_print_hex(HW_USBCTRL_USBSTS_RD());
debug_print_string("\r\n");

for (;;);				// loop forever
}

