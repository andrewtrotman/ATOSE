/*
	IMX6Q_USB.C
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Thanks to Nick Sherlock for working wit me on the predecessor of this, for the i.MX233

	Make the i.MX6Q look like a USB CDC device (serial port) and then echo back any data recieved.
	To test this program on my Mac I do the following....
		Open a terminal window on the UART (minicom -D /dev/tty.usbserial-FTG4ND95)
		Upload this program to the SABRE Lite (imx_run.mac imx6q_usb.elf)
		You'll get a whole load of debug through the UART
		Open a terminal window on the USB CDC we just created (minicom -D /dev/tty.usbmodemSN#DE1)
		Now I get local echo on the USB CDC device (i.e the terminal window above).
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
	We need to tell the i.MX6 SDK that we're using an i.MX6Q
*/
#define CHIP_MX6DQ 1

/*
	i.MX6 SDK include files
*/
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsuart.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsccm.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsiomuxc.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsgpt.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbcore.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsusbphy.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/registers/regsepit.h"
#include "../systems/iMX6_Platform_SDK/sdk/include/mx6dq/irq_numbers.h"


/*
	Default baud rate is 115200 and we'll talk down UARR-2 as that's the
	console on the SABRE Lite board
*/
#define BAUD_RATE 115200
#define DEFAULT_UART 2		/* can be either 2 (SABRE Lite "console") or 1 (the other UART) */
#define PLL3_FREQUENCY 80000000			// the frequency of Phase Lock Loop 3

/*
	We're compatible with: USB 2.0; CDC 1.1; Microsoft Extensions version 1.0
	Recall that all directions are RELATIVE TO THE HOST.
		OUT = from the host (i.e. PC)
		IN  = to the host (i.e. PC)

		High Speed :480 Mbps
		Full Speed: 12Mbps
		Low Speed: 1.5Mbps
*/
#define USB_BCD_VERSION 						0x0200		/* we're compliant with USB 2.0 */
#define USB_CDC_BCD_VERSION					0x0110		// version 1.1 of the CDC spec */
#define MS_USB_VERSION							0x0100

/*
	The various bits in the bmRequestType member of a setup packet
	See page 248 of "Universal Serial Bus Specification Revision 2.0"
*/
#define USB_SETUP_DIRECTION_HOST_TO_DEVICE		0x00
#define USB_SETUP_DIRECTION_DEVICE_TO_HOST		0x10
#define USB_SETUP_TYPE_STANDARD 				0x00
#define USB_SETUP_TYPE_CLASS 					0x01
#define USB_SETUP_TYPE_VENDOR 					0x02
#define USB_SETUP_TYPE_RESERVED 				0x03
#define USB_SETUP_RECIPIENT_DEVICE 			0x00
#define USB_SETUP_RECIPIENT_INTERFACE 			0x01
#define USB_SETUP_RECIPIENT_ENDPOINT 			0x02
#define USB_SETUP_RECIPIENT_OTHER 				0x03

/*
	States that the USB device hardware can be in
	See page 243 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
*/
#define USB_DEVICE_STATE_ATTACHED				0x00
#define USB_DEVICE_STATE_POWERED				0x01
#define USB_DEVICE_STATE_DEFAULT				0x02
#define USB_DEVICE_STATE_ADDRESS				0x03
#define USB_DEVICE_STATE_CONFIGURED			0x04
#define USB_DEVICE_STATE_SUSPENDED				0x05

/*
	Product and manufacturer IDs (allocated by the USB organisation)
*/
#define USB_ID_VENDOR							0xDEAD		// our vendor ID
#define USB_ID_PRODUCT 						0x000E		// device number
#define USB_ID_DEVICE							0x0100		// revision number

/*
	Configurable parameters
*/
#define USB_MAX_PACKET_SIZE					64			// under USB 2.0 this must be 8, 16, 32, or 64

/*
	String constants
*/
#define USB_STRING_NONE						0x00
#define USB_STRING_LANGUAGE					0x00		// See page 273 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
#define USB_STRING_MANUFACTURER				0x01
#define USB_STRING_PRODUCT						0x02
#define USB_STRING_SERIAL_NUMBER				0x03
#define MS_USB_STRING_OS_DESCRIPTOR			0xEE		// See page 6 of "Microsoft OS Descriptors Overview"

/*
	language codes
*/
#define USB_LANGUAGE_ENGLISH_UNITED_STATES		0x0409
#define USB_LANGUAGE_ENGLISH_UNITED_KINGDOM	0x0809
#define USB_LANGUAGE_ENGLISH_AUSTRALIAN		0x0c09
#define USB_LANGUAGE_ENGLISH_CANADIAN			0x1009
#define USB_LANGUAGE_ENGLISH_NEW_ZEALAND		0x1409
#define USB_LANGUAGE_ENGLISH_IRELAND			0x1809
#define USB_LANGUAGE_ENGLISH_SOUTH_AFRICA		0x1c09
#define USB_LANGUAGE_ENGLISH_JAMAICA			0x2009
#define USB_LANGUAGE_ENGLISH_CARIBBEAN			0x2409
#define USB_LANGUAGE_ENGLISH_BELIZE			0x2809
#define USB_LANGUAGE_ENGLISH_TRINIDAD			0x2c09
#define USB_LANGUAGE_ENGLISH_ZIMBABWE			0x3009
#define USB_LANGUAGE_ENGLISH_PHILIPPINES		0x3409

/*
	The different descriptor types we know about
*/
#define USB_DESCRIPTOR_TYPE_DEVICE						0x01
#define USB_DESCRIPTOR_TYPE_CONFIGURATION				0x02
#define USB_DESCRIPTOR_TYPE_STRING						0x03
#define USB_DESCRIPTOR_TYPE_INTERFACE					0x04
#define USB_DESCRIPTOR_TYPE_ENDPOINT					0x05
#define USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER			0x06
#define USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION	0x07
#define USB_DESCRIPTOR_TYPE_INTERFACE_POWER			0x08


#define USB_DESCRIPTOR_TYPE_CS_INTERFACE		0x24

#define MS_USB_DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID  0x04
#define MS_USB_DESCRIPTOR_TYPE_EXTENDED_PROPERTIES 0x05

/*
	The different descriptor subtypes we know about
*/
#define USB_DESCRIPTOR_SUBTYPE_HEADER			0x00
#define USB_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT 0x01
#define USB_DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT 0x02
#define USB_DESCRIPTOR_SUBTYPE_UNION_FUNCTION	0x06

/*
	The different device classes we know about
*/
#define USB_DEVICE_CLASS_CDC					0x02

/*
	The different interface classes we know about
*/
#define USB_INTERFACE_CLASS_CDC				0x02
#define USB_INTERFACE_CLASS_DATA				0x0A

/*
	The different device subclasses we know about
*/
#define USB_DEVICE_SUBCLASS_NONE 				0x00

/*
	The different device protocols we know about
*/
#define USB_DEVICE_PROTOCOL_NONE				0x00

/*
	Configurations
*/
#define USB_CONFIGURATION_SELFPOWERED 			0xC0
#define USB_CONFIGURATION_REMOTEWAKE  			0xA0
#define USB_CONFIGURATION_100mA				50

/*
	Endpoint directions
*/
#define USB_ENDPOINT_DIRECTION_IN				0x80
#define USB_ENDPOINT_DIRECTION_OUT				0x00

/*
	Endpoint types
*/
#define USB_ENDPOINT_TYPE_CONTROL				0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS			0x01
#define USB_ENDPOINT_TYPE_BULK					0x02
#define USB_ENDPOINT_TYPE_INTERRUPT			0x03

/*
	Standard USB methods on endpoint 0
	See page 251 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
*/
#define USB_REQUEST_GET_STATUS 				0x00
#define USB_REQUEST_CLEAR_FEATURE 				0x01
//#define USB_REQUEST_RESERVED					0x02
#define USB_REQUEST_SET_FEATURE 				0x03
//#define USB_REQUEST_RESERVED					0x04
#define USB_REQUEST_SET_ADDRESS 				0x05
#define USB_REQUEST_GET_DESCRIPTOR 			0x06
#define USB_REQUEST_SET_DESCRIPTOR 			0x07
#define USB_REQUEST_GET_CONFIGURATION 			0x08
#define USB_REQUEST_SET_CONFIGURATION 			0x09
#define USB_REQUEST_GET_INTERFACE 				0x0A
#define USB_REQUEST_SET_INTERFACE 				0x0B
#define USB_REQUEST_SYNCH_FRAME 				0x0C

/*
	USB features
*/
#define USB_FEATURE_ENDPOINT_HALT				0x00
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP		0x01
#define USB_FEATURE_TEST_MODE					0x02

/*
	USB CDC parameters
*/
#define USB_CDC_ABSTRACT_CONTROL 				0x02
#define USB_CDC_PROTOCOL_HAYES 				0x01
#define USB_CDC_PROTOCOL_NONE 					0x00

/*
	USB CDC Capabilities:Call Management
*/
#define USB_CDC_CAPABILITY_NONE 				0x00
#define USB_CDC_CAPABILITY_CM_COMUNICATIONS	0x01
#define USB_CDC_CAPABILITY_CM_DATA				0x03

/*
	USB CDC Capabilities:Abstract Control Management
*/
#define USB_CDC_CAPABILITY_CONNECT				0x08			// supports Network_Connection
#define USB_CDC_CAPABILITY_BREAK       		0x04			// supports Send_Break
#define USB_CDC_CAPABILITY_LINE       			0x02			// supports Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, Serial_State
#define USB_CDC_CAPABILITY_FEATURE      		0x01			// supports Set_Comm_Feature, Clear_Comm_Feature, Get_Comm_Feature

/*
	USB CDC Virtual COM port endpoint allocation
*/
#define USB_CDC_ENDPOINT_CONTROL 						0 		// Endpoint 0 is always control
#define USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT 	1		// Endpoint 1 is control management to the host
#define USB_CDC_ENDPOINT_SERIAL						2		// Endpoint 2 is for data

/*
	USB CDC Line parameters
*/

#define USB_CDC_STOP_BITS_1					0x00
#define USB_CDC_STOP_BITS_15					0x01
#define USB_CDC_STOP_BITS_2					0x02

#define USB_CDC_PARITY_NONE					0x00
#define USB_CDC_PARITY_ODD						0x01
#define USB_CDC_PARITY_EVEN					0x02
#define USB_CDC_PARITY_MARK					0x03
#define USB_CDC_PARITY_SPACE					0x04

/*
	Messages from the host to the device
*/
#define USB_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND	0x00
#define USB_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE	0x01
#define USB_CDC_REQUEST_SET_COMM_FEATURE			0x02
#define USB_CDC_REQUEST_GET_COMM_FEATURE			0x03
#define USB_CDC_REQUEST_CLEAR_COMM_FEATURE			0x04
#define USB_CDC_REQUEST_SET_LINE_CODING			0x20
#define USB_CDC_REQUEST_GET_LINE_CODING			0x21
#define USB_CDC_REQUEST_SET_CONTROL_LINE_STATE		0x22
#define USB_CDC_REQUEST_SEND_BREAK					0x23

/*
	Messages from the device to the host
*/
#define USB_CDC_NOTIFICATION_NETWORK_CONNECTION	0x00
#define USB_CDC_NOTIFICATION_RESPONSE_AVAILABLE	0x01
#define USB_CDC_NOTIFICATION_SERIAL_STATE			0x20

/*
	Microsoft specific
*/
#define MS_USB_REG_SZ 							0x01

#define MS_USB_REQUEST_GET_EXTENDED_COMPAT_ID_OS_FEATURE_DESCRIPTOR 0xC0
#define MS_USB_REQUEST_GET_EXTENDED_PROPERTIES_OS_DESCRIPTOR 0xC1

/*
	i.MX specific
*/
#define IMX_ENDPOINT_TRANSFER_DESCRIPTOR_BUFFER_POINTERS	5
#define IMX_ENDPOINT_TRANSFER_DESCRIPTOR_TERMINATOR 		((imx_endpoint_transfer_descriptor *)0x01)
#define IMX_ENDPOINT_TRANSFER_DESCRIPTOR_STATUS_ACTIVE 	0x80

/*
	a transfer can do at most 4096 bytes of memory per buffer (due to the way counting is done).
	See page 7 of "Simplified Device Data Structures for the High-End ColdFire Family USB Modules"
*/
#define IMX_QUEUE_BUFFER_SIZE 4096

/*
	There are i.MX233 queue heads
*/
/*
	The i.MX53 and i.MX6Q have 8 endpoints 0..7
		see page 5449 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
		see page 4956 of "i.MX53 Multimedia Applications Processor Reference Manual Rev. 2.1, 06/2012"
*/
#define IMX_USB_MAX_ENDPOINTS 8

/*
	Queue heads must lie in internal memory for controller speed requirements.
	Lower 11 bits of address cannot be set so we must have a 2K alignment.

	Queuehead 0: OUT (USB Control from host)
	Queuehead 1: IN  (USB Control to host)
	Queuehead 2: OUT (Abstract Control Management from host) 		NOT USED
	Queuehead 3: IN (Abstract Control Management to host)
	Queuehead 4: OUT (Data from host)
	Queuehead 5: IN  (Data to host)
*/
#define IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT	0			// even numbers are OUT/SETUP		(see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012")
#define IMX_USB_CDC_QUEUEHEAD_CONTROL_IN 	1			// odd numbers are IN				(see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012")
#define IMX_USB_CDC_QUEUEHEAD_ACM_OUT		2	// NOT USED
#define IMX_USB_CDC_QUEUEHEAD_ACM_IN		3
#define IMX_USB_CDC_QUEUEHEAD_DATA_OUT		4
#define IMX_USB_CDC_QUEUEHEAD_DATA_IN		5

/*
	=========
	USB stuff
	=========
*/

/*
	union USB_SETUP_DATA_REQUEST_TYPE
	---------------------------------
	Page 248 of "Universal Serial Bus Specification Revision 2.0"
	This union is used to decode the meaning of the bmRequestType memeber of
	a setup packet (see declaration of usb_setup_data)
*/
typedef union
{
uint8_t all;
struct
	{
	unsigned recipient: 5;
	unsigned type : 2;
	unsigned direction : 1;
	} __attribute__ ((packed)) bit;
} usb_setup_data_request_type;

/*
	struct USB_SETUP_DATA
	---------------------
	Page 248 of "Universal Serial Bus Specification Revision 2.0"
	This structure is the shape of the setup packet sent from the host to the device.
	It is used in communications before the device comes on line
*/
typedef struct
{
usb_setup_data_request_type bmRequestType;
uint8_t bRequest;
uint16_t wValue;
uint16_t wIndex;
uint16_t wLength;
} __attribute__ ((packed)) usb_setup_data;

/*
	struct USB_STANDARD_DEVICE_DESCRIPTOR
	-------------------------------------
	Page 262-263 of "Universal Serial Bus Specification Revision 2.0"
	This structure is used by the device to tell the host what the device is
*/
typedef struct
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
} __attribute__ ((packed)) usb_standard_device_descriptor;

/*
	struct USB_DEVICE_QUALIFIER_DESCRIPTOR
	--------------------------------------
	Page 262-263 of "Universal Serial Bus Specification Revision 2.0"
	This gets used when a high-speed device is plugged in so that the host
	can determine what would happen if plugged into a non-high-speed port.
*/
typedef struct
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
} __attribute__ ((packed)) usb_device_qualifier_descriptor;

/*
	struct USB_STRING_DESCRIPTOR_LANGUAGE
	-------------------------------------
	Page 273 of "Universal Serial Bus Specification Revision 2.0"
	The host will ask us what language our text strings are in, the device
	responds with one of these.  As some devices support multiple languages
	the response can include several languages.  We'll only worry about one
	language at the moment.
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t wLANGID;
}  __attribute__ ((packed)) usb_string_descriptor_language;


/*
	struct USB_STRING_DESCRIPTOR
	----------------------------
	Page 273 of "Universal Serial Bus Specification Revision 2.0"
	When we return a string to the host its in one of these objects
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t wString[];
}  __attribute__ ((packed)) usb_string_descriptor;

/*
	struct USB_STANDARD_CONFIGURATION_DESCRIPTOR
	--------------------------------------------
	Page 265 of "Universal Serial Bus Specification Revision 2.0"
	A device may have several configurations, the host selects which one to use
	by calling SetConfiguration() using the bConfigurationValue value

*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint16_t wTotalLength;
uint8_t bNumInterfaces;
uint8_t bConfigurationValue;
uint8_t iConfiguration;
uint8_t bmAttributes;
uint8_t bMaxPower;
} __attribute__ ((packed)) usb_standard_configuration_descriptor;

/*
	struct USB_STANDARD_INTERFACE_DESCRIPTOR
	----------------------------------------
	Page 268-269 of "Universal Serial Bus Specification Revision 2.0"
	Each configuration has one or more interfaces.
*/
typedef struct
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
} __attribute__ ((packed)) usb_standard_interface_descriptor;


/*
	struct USB_STANDARD_ENDPOINT_DESCRIPTOR
	---------------------------------------
	Page 269-271 of "Universal Serial Bus Specification Revision 2.0"
	Each interface is attached to some number of endpoints.  An endpoint is
	just like a socket - it is where we speak in order to pass a message around.
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bEndpointAddress;			// the endpont number
uint8_t bmAttributes;
uint16_t wMaxPacketSize;
uint8_t bInterval;
} __attribute__ ((packed)) usb_standard_endpoint_descriptor;


/*
	=============
	USB CDC stuff
	=============
*/
/*
	struct USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR
	-------------------------------------------
	Page 34 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint16_t bcdCDC;
} __attribute__ ((packed)) usb_cdc_header_functional_descriptor;

/*
	struct USB_CDC_CALL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	----------------------------------------------------
	Page 34-35 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint8_t bmCapabilities;
uint8_t bDataInterface;
} __attribute__ ((packed)) usb_cdc_call_management_functional_descriptor;

/*
	struct USB_CDC_ABSTRACT_CONTROL_MANAGEMENT_FUNCTIONAL_DESCRIPTOR
	----------------------------------------------------------------
	Page 35-36 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint8_t bmCapabilities;
} __attribute__ ((packed)) usb_cdc_abstract_control_management_functional_descriptor;

/*
	struct USB_UNION_INTERFACE_FUNCTIONAL_DESCRIPTOR
	------------------------------------------------
	Page 40-41 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t bDescriptorSubType;
uint8_t bMasterInterface;
uint8_t bSlaveInterface0;
} __attribute__ ((packed)) usb_union_interface_functional_descriptor;

/*
	stuct USB_CDC_VIRTUAL_COM_PORT
	------------------------------
	A virtual COM port has one configuration, two interfaces (data and control) and three
	endpoints (data in and data out count as two).
*/

typedef struct
{
usb_standard_configuration_descriptor cd;
	usb_standard_interface_descriptor id0;
		usb_cdc_header_functional_descriptor fd1;
		usb_cdc_call_management_functional_descriptor fd2;
		usb_cdc_abstract_control_management_functional_descriptor fd3;
		usb_union_interface_functional_descriptor fd4;
			usb_standard_endpoint_descriptor ep2;
	usb_standard_interface_descriptor id1;
		usb_standard_endpoint_descriptor ep3;
		usb_standard_endpoint_descriptor ep4;
} __attribute__ ((packed)) usb_cdc_virtual_com_port;

/*
	struct USB_CDC_LINE_CODING
	--------------------------
*/
typedef struct
{
uint32_t dwDTERat;		// Number Data terminal rate, in bits per second.
uint8_t bCharFormat;	// Number Stop bits (0 = 1 Stop bit, 1 = 1.5 Stop bits, 2 = 2 Stop bits)
uint8_t bParityType;	// Number Parity (0 = None, 1 = Odd, 2 = Even, 3 = Mark, 4 = Space)
uint8_t bDataBits;		// Number Data bits (5, 6, 7, 8 or 16).
} __attribute__ ((packed)) usb_cdc_line_coding;

/*
	=========================================
	Microsoft extensions to the USB protocols
	=========================================
*/

/*
	struct USB_MS_OS_STRING_DESCRIPTOR
	----------------------------------
	Page 7 of "Microsoft OS Descriptors Overview (July 13, 2012)"
*/
typedef struct
{
uint8_t bLength;
uint8_t bDescriptorType;
uint8_t qwSignature[14];
uint8_t bMS_VendorCode;
uint8_t bPad;
} __attribute__ ((packed)) ms_usb_os_string_descriptor;

/*
	struct MS_USB_COMPATIBLE_ID_HEADER
	----------------------------------
	Page 6 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
*/
typedef struct
{
uint32_t dwLength;
uint16_t bcdVersion;
uint16_t wIndex;
uint8_t bCount;
uint8_t reserved[7];
} __attribute__ ((packed)) ms_usb_compatible_id_header;

/*
	struct MS_USB_COMPATIBLE_ID_FUNCTION
	------------------------------------
	Page 7 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
*/
typedef struct
{
uint8_t bFirstInterfaceNumber;
uint8_t reserved1;
uint8_t compatibleID[8];
uint8_t subCompatibleID[8];
uint8_t reserved2[6];
} __attribute__ ((packed)) ms_usb_compatible_id_function;

/*
	struct MS_USB_EXTENDED_COMPATIBLE_ID_OS_FEATURE_DESCRIPTOR
	----------------------------------------------------------
	Page 6 of "Extended Compat ID OS Feature Descriptor Specification (July 13, 2012)"
	This structure can hold manty different ms_usb_function_section objects, but for
	what we need one is enough.
*/
typedef struct
{
ms_usb_compatible_id_header header;
ms_usb_compatible_id_function section1;
} __attribute__ ((packed)) ms_usb_extended_compatible_id_os_feature_descriptor;

/*
	struct MS_USB_EXTENDED_PROPERTY_HEADER
	--------------------------------------
	Page 6 of "Extended Properties OS Feature Descriptor Specification (July 13, 2012)"
*/
typedef struct
{
uint32_t dwLength;				// sizeof whole descriptor
uint16_t bcdVersion;
uint16_t wIndex;
uint16_t wCount;
} __attribute__ ((packed)) ms_usb_extended_property_header;

/*
	struct MS_USB_CUSTOM_PROPERTY_FUNCTION
	--------------------------------------
	Page 7 of "Extended Properties OS Feature Descriptor Specification (July 13, 2012)"
*/
typedef struct
{
uint32_t dwSize;
uint32_t dwPropertyDataType;
uint16_t wPropertyNameLength;
uint8_t bPropertyName[12];				// "Label" in Unicode
uint32_t dwPropertyDataLength;
uint8_t bPropertyData[16];
} __attribute__ ((packed)) ms_usb_extended_property_function;


/*
	struct USB_MS_EXTENDED_PROPERTIES
	---------------------------------
	Page 6 of "Extended Properties OS Feature Descriptor Specification (July 13, 2012)"
	This can have several properties, but we only need one.
*/
typedef struct
{
ms_usb_extended_property_header header;
ms_usb_extended_property_function property1;
}  __attribute__ ((packed)) ms_usb_extended_properties;

/*
	=============
	i.MX6Q stuff
	=============
*/

/*
	union IMX_ENDPOINT_TRANSFER_DESCRIPTOR_DTD_TOKEN
	------------------------------------------------
*/
typedef union
{
uint32_t all;
struct
	{
	unsigned status : 8;
	unsigned reserved0 : 2;
	unsigned mult0 : 2;
	unsigned reserved1 : 3;
	unsigned ioc : 1;
	unsigned total_bytes : 15;
	unsigned reserved2 : 1;
	} bit __attribute__ ((packed));
} imx_endpoint_transfer_descriptor_dtd_token;

/*
	struct IMX_ENDPOINT_TRANSFER_DESCRIPTOR
	---------------------------------------
	Page 5332 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	The Endpoint Transfer Descriptor data struct must be 32-bye aligned (and is 32-bytes long) hence the padding at the end
*/
typedef struct imx_endpoint_transfer_descriptor_
{
struct imx_endpoint_transfer_descriptor_ *next_link_pointer;		/* Next TD pointer(31-5), T(0) set indicate invalid */
imx_endpoint_transfer_descriptor_dtd_token token;
void *buffer_pointer[IMX_ENDPOINT_TRANSFER_DESCRIPTOR_BUFFER_POINTERS];
uint32_t reserved;							/* Reserved */
} __attribute__ ((packed)) imx_endpoint_transfer_descriptor;

/*
	union IMX_ENDPOINT_QUEUEHEAD_CAPABILITIES
	-----------------------------------------
*/
typedef union
{
uint32_t all;
struct
	{
	unsigned reserved0 : 15;
	unsigned ios : 1;
	unsigned maximum_packet_length : 11;
	unsigned reserved1 : 2;
	unsigned zlt : 1;
	unsigned mult : 2;							// not sure if this exists on the i.MX233
	} bit __attribute__ ((packed));
} imx_endpoint_queuehead_capabilities;

/*
	struct IMX_ENDPOINT_QUEUEHEAD
	-----------------------------
	Page 5330 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	This structure must be 64-byte aligned.  To make sure an array of
	these is correctly alligned its necessary to pad the end of the
	structure with a few extra words
*/
typedef struct
{
imx_endpoint_queuehead_capabilities capabilities;			/* capability information about the endpoint */
imx_endpoint_transfer_descriptor *current_dtd_pointer;		/* Current dTD Pointer(31-5). This is set by the USB controller */
imx_endpoint_transfer_descriptor dtd_overlay_area;			/* The hardware copies the current transfer descriptor here when it actions it. */
usb_setup_data setup_buffer;								/* Setup packets are copies here rather than into the transfer descriptor buffers */
uint32_t reserved2[4];										/* Needed to guarantee 64-byte allignment */
} __attribute__ ((packed)) imx_endpoint_queuehead;



/*
	================================
	Now for some of our constants...
	================================
*/

/*
	OUR_ZERO
	--------
	for transmitting to the host
*/
uint8_t our_zero_8 = 0;
uint16_t our_zero_16 = 0;
uint32_t our_zero_32 = 0;

/*
	OUR_ONE
	-------
	for transmitting to the host
*/
uint8_t our_one_8 = 1;
uint16_t our_one_16 = 1;
uint32_t our_one_32 = 1;

/*
	OUR_DEVICE_DESCRIPTOR
	---------------------
	We're going top fake being a serial port
*/
usb_standard_device_descriptor our_device_descriptor =
{
.bLength = sizeof(our_device_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
.bcdUSB = USB_BCD_VERSION,						// USB 2.0
.bDeviceClass = USB_DEVICE_CLASS_CDC, 			// Communications Device Class (Pretend to be a Serial Port)
.bDeviceSubClass = USB_DEVICE_SUBCLASS_NONE,
.bDeviceProtocol = USB_DEVICE_PROTOCOL_NONE,
.bMaxPacketSize0 = USB_MAX_PACKET_SIZE, 		// Accept 64 bytes packet size on control endpoint (the maximum)
.idVendor = USB_ID_VENDOR,						// Manufacturer's ID
.idProduct = USB_ID_PRODUCT,					// Product's ID
.bcdDevice = USB_ID_DEVICE,					// Product Verison Number (revision number)
.iManufacturer = USB_STRING_MANUFACTURER,		// string ID of the manufacturer
.iProduct = USB_STRING_PRODUCT,				// string ID of the Product
.iSerialNumber = USB_STRING_SERIAL_NUMBER,		// string ID of the serial number
.bNumConfigurations = 0x01
};

/*
	OUR_DEVICE_QUALIFIER
	--------------------
*/
usb_device_qualifier_descriptor our_device_qualifier =
{
.bLength = sizeof(our_device_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
.bcdUSB = USB_BCD_VERSION, 				// USB 2.0
.bDeviceClass = USB_DEVICE_CLASS_CDC, 			// Communications Device Class (Pretend to be a Serial Port)
.bDeviceSubClass = USB_DEVICE_SUBCLASS_NONE,
.bDeviceProtocol = USB_DEVICE_PROTOCOL_NONE,
.bMaxPacketSize0 = USB_MAX_PACKET_SIZE, 		// Accept 64 bytes packet size on control endpoint (the maximum)
.bNumConfigurations = 0x01,
.reserved = 0x00
} ;


/*
	OUR_LANGUAGE
	------------
*/
usb_string_descriptor_language our_language =
{
.bLength = sizeof(our_language),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wLANGID = USB_LANGUAGE_ENGLISH_NEW_ZEALAND
};

/*
	OUR_SERIAL_NUMBER
	-----------------
*/
usb_string_descriptor our_serial_number =
{
.bLength = 14,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'S', 0x00, 'N', 0x00, '#', 0x00, 'D', 0x00, 'E', 0x00, 'V', 0x00}
} ;

/*
	OUR_MANUFACTURER
	----------------
*/
usb_string_descriptor our_manufacturer =
{
.bLength = 10,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'A', 0x00, 'S', 0x00, 'P', 0x00, 'T', 0x00}
} ;

/*
	OUR_PRODUCT
	-----------
*/
usb_string_descriptor our_product =
{
.bLength = 16,
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'F', 0x00, 'o', 0x00, 'u', 0x00, 'r', 0x00, 'A', 0x00, 'R', 0x00, 'M', 0x00}
} ;

/*

	I based this on the USB Serial Example for Teensy USB Development
	Board (http://www.pjrc.com/teensy/usb_serial.html) but as its
	completely re-written and isn't "substantial portions of the Software"
	and that software is under a BSD-like licence, I don't reproduce their
	copyright notice here (go see it yourself if you really want to read it).

	The version of the CDC spec being referenced is "Universal Serial Bus
	Class Definitions for Communication Devices Version 1.1 January 19,
	1999"

	The version of the USB spec being referenced is "Universal Serial Bus
	Specification Revision 2.0 April 27, 2000"

	Interface 0:
		Abstract Control Management
			Endpoint 2 (IN to host) Interrupt
	Interface 1:
		Data
			Endpoint 3 (OUT from host) Bulk
			Endpoint 4 (IN to host)  Bulk
*/
usb_cdc_virtual_com_port our_com_descriptor =
{
	{
	/*
		Configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	*/
	.bLength = sizeof(usb_standard_configuration_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION,
	.wTotalLength = sizeof(usb_cdc_virtual_com_port),
	.bNumInterfaces = 2,												// we have 2 interfaces: Abstract control management; and data
	.bConfigurationValue = 1,											// we are configuration 1
	.iConfiguration = USB_STRING_NONE,
	.bmAttributes = USB_CONFIGURATION_SELFPOWERED,						// selfpowered
	.bMaxPower = USB_CONFIGURATION_100mA								// we draw no more than 100mA
	},
	{
	/*
		Interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	*/
	.bLength = sizeof(usb_standard_interface_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
	.bInterfaceNumber = 0,												// control on interface 0
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,													// uses 1 endpoint
	.bInterfaceClass = USB_INTERFACE_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_ABSTRACT_CONTROL,						// Abstract Control
	.bInterfaceProtocol = USB_CDC_PROTOCOL_HAYES,						// using Hayes AT command set
	.iInterface = 0
	},
	{
	/*
		CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
	*/
	.bLength = sizeof(usb_cdc_header_functional_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_CS_INTERFACE,
	.bDescriptorSubType = USB_DESCRIPTOR_SUBTYPE_HEADER,
	.bcdCDC = USB_CDC_BCD_VERSION
	},
	{
	/*
		Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
	*/
	.bLength = sizeof(usb_cdc_call_management_functional_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_CS_INTERFACE,
	.bDescriptorSubType = USB_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT,
	.bmCapabilities = USB_CDC_CAPABILITY_CM_COMUNICATIONS,				// we do call management over the Communicatons Class Interface
	.bDataInterface = 1													// data class interface is interface 1
	},
	{
	/*
		Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
	*/
	.bLength = sizeof(usb_cdc_abstract_control_management_functional_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_CS_INTERFACE,
	.bDescriptorSubType = USB_DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT,
	.bmCapabilities = USB_CDC_CAPABILITY_BREAK | USB_CDC_CAPABILITY_LINE	// we must manage Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, Serial_State, and Send_Break
	},
	{
	/*
		Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
	*/
	.bLength = sizeof(usb_union_interface_functional_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_CS_INTERFACE,
	.bDescriptorSubType = USB_DESCRIPTOR_SUBTYPE_UNION_FUNCTION,
	.bMasterInterface = 0,												// interface 0 is for control
	.bSlaveInterface0 = 1												// interface 1 is for other stuff
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	.bLength = sizeof(usb_standard_endpoint_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
	.bEndpointAddress = USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT | USB_ENDPOINT_DIRECTION_IN,
	.bmAttributes = USB_ENDPOINT_TYPE_INTERRUPT,						// notifications on an INTERRUPT ENDPOINT
	.wMaxPacketSize = USB_MAX_PACKET_SIZE,
	.bInterval = 10
	},
	{
	/*
		Interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	*/
	.bLength = sizeof(usb_standard_interface_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
	.bInterfaceNumber = 1,												// data on interface 1
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,													// uses 2 endpoints
	.bInterfaceClass = USB_INTERFACE_CLASS_DATA,						// data
	.bInterfaceSubClass = USB_DEVICE_SUBCLASS_NONE,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
	.iInterface = 0
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	.bLength = sizeof(usb_standard_endpoint_descriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_ENDPOINT,
	.bEndpointAddress = USB_CDC_ENDPOINT_SERIAL | USB_ENDPOINT_DIRECTION_OUT,
	.bmAttributes = USB_ENDPOINT_TYPE_BULK,
	.wMaxPacketSize = USB_MAX_PACKET_SIZE,
	.bInterval = 0														// this endpoint does not NAK
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	.bLength = sizeof(usb_standard_endpoint_descriptor),
	.bDescriptorType = 	USB_DESCRIPTOR_TYPE_ENDPOINT,
	.bEndpointAddress = USB_CDC_ENDPOINT_SERIAL | USB_ENDPOINT_DIRECTION_IN,
	.bmAttributes = USB_ENDPOINT_TYPE_BULK,
	.wMaxPacketSize = USB_MAX_PACKET_SIZE,
	.bInterval = 0														// this endpoint does not NAK
	}
};

/*
	===================
	USB CDC descriptors
	===================
*/

/*
	OUR_CDC_LINE_CODING
	-------------------
	115200,n,8,1
*/
usb_cdc_line_coding our_cdc_line_coding =
{
.dwDTERat = 115200,
.bCharFormat = USB_CDC_STOP_BITS_1,
.bParityType =  USB_CDC_PARITY_NONE,
.bDataBits =  8
} ;

/*
	================================
	Microsoft extensions descriptors
	================================
*/
/*
	OUR_MS_OS_STRING_DESCRIPTOR
	---------------------------
*/
ms_usb_os_string_descriptor our_ms_os_string_descriptor =
{
.bLength = sizeof(our_ms_os_string_descriptor),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.qwSignature = {'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, '1', 0x00, '0', 0x00, '0', 0x00},		// MSFT100
.bMS_VendorCode = 0,																				// this is a user-defined value and appears to be pretty much useless
.bPad = 0
};


/*
	OUR_MS_COMPATIBLE_ID_FEATURE_DESCRIPTOR
	---------------------------------------
*/
ms_usb_extended_compatible_id_os_feature_descriptor our_ms_compatible_id_feature_descriptor =
{
	{
	.dwLength = sizeof(our_ms_compatible_id_feature_descriptor),
	.bcdVersion = MS_USB_VERSION,
	.wIndex = MS_USB_DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID,
	.bCount = 1,
	.reserved = {0},
	},
	{
	.bFirstInterfaceNumber = 0,
	.reserved1 = 0,
	.compatibleID = {0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00},	// WINUSB
	.subCompatibleID = {0},
	.reserved2 = {0}
	}
} ;


/*
	OUR_MS_EXTENDED_PROPERTIES
	--------------------------
*/
ms_usb_extended_properties our_ms_extended_properties =
{
	{
	.dwLength = sizeof(our_ms_extended_properties),
	.bcdVersion = MS_USB_VERSION,
	.wIndex = MS_USB_DESCRIPTOR_TYPE_EXTENDED_PROPERTIES,
	.wCount = 1															// 1 custom property
	},
	{
	.dwSize = sizeof(ms_usb_extended_property_function),
	.dwPropertyDataType = MS_USB_REG_SZ,
	.wPropertyNameLength = 12,
	.bPropertyName = {'L', 0x00, 'a', 0x00, 'b', 0x00, 'e', 0x00, 'l', 0x00, 0x00, 0x00},
	.dwPropertyDataLength = 16,
	.bPropertyData = {'F', 0x00, 'o', 0x00, 'u', 0x00, 'r', 0x00, 'A', 0x00, 'R', 0x00, 'M', 0x00, 0x00, 0x00}
	}
} ;


/*
	=====================
	Now on to the i.MX6Q
	=====================
*/

/*
	There are two queue heads for each endpoint, even numbers are OUT going from host, odd numbers are IN coming to host
	"The Endpoint Queue Head List must be aligned to a 2k boundary"  see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
imx_endpoint_queuehead global_queuehead[IMX_USB_MAX_ENDPOINTS * 2] __attribute__ ((aligned (2048)));

/*
	Each endpoint has a transfer descriptor
	Endpoint transfer descriptors must be aligned on 32-byte boundaries because the last 5 bits must be zero
	Also note that the last bit is a termination bit (T), see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
imx_endpoint_transfer_descriptor global_dTD[IMX_USB_MAX_ENDPOINTS * 2] __attribute__ ((aligned (32)));

/*
	Each transfer descriptor has a pointer to 5 separate transfer buffers. We're only going to use one of these but we'll
	allocate the space to keep all the pointers in case we need more

	"Buffer Pointer. Indicates the physical memory address for the data buffer to be used by the
	dTD. The device controller actually uses the first part of the address (bits 31-12) as a
	pointer to a 4 KB page, and the lower part of the address (bits 11-0) as an index into the
	page. The host controller will increment the index internally, but will not increment the page
	address. This is what determines the 4 KB transfer size limitation used for this application
	note." see page 7 of "Simplified Device Data Structures for the High-End ColdFire Family USB Modules"
*/
uint8_t *global_transfer_buffer[IMX_USB_MAX_ENDPOINTS * 2][IMX_ENDPOINT_TRANSFER_DESCRIPTOR_BUFFER_POINTERS];

/*
	This is used to prevent re-allocation of buffers if the device is disconnected and then reconnected
*/
uint32_t global_transfer_buffers_preallocated;


/*
	Forward declaration
*/
void usb_setup_endpoint_nonzero(void);
uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

/*
	=======================
	On to the generic stuff
	=======================
*/

/*
	There are five states a USB device can be in (below) but we really only need to worry about three of these,
		the DEFAULT, ADDRESS, and CONFIGURED states.  Later we might worry about the SUSPEND state.  The states
		are:

		ATTACHED, "A USB device may be attached or detached from the USB. The state of a USB device when
		it is detached from the USB is not defined by this specification. This specification only addresses
		required operations and attributes once the device is attached." Page 242 of "Universal Serial Bus
		Specification Revision 2.0 April 27, 2000". We don't need to deal with this state as the hardware
		does it for us.

		POWERED, "Although self-powered devices may already be powered before they are attached to the USB,
		they are not considered to be in the Powered state until they are attached to the USB and VBUS is
		applied to the device." Page 242 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		As with the ATTACHED state, I doubt we need to worry about this other than to make sure that on startup
		the device driver is put into the DEFAULT state (below).

		DEFAULT, in which the device is communicating over port 0 and looking for an address, this is
		essentially a setup phase in which the device identifies itself but is not permitted to respond
		to requests to perform actions.  Page 242 of "Universal Serial Bus Specification Revision 2.0
		April 27, 2000" describes it as "After the device has been powered, it must not respond to any
		bus transactions until it has received a reset from the bus. After receiving a reset, the device
		is then addressable at the default address."  The default address is 0 and the default endpoint
		is 0.  To move to the next state (ADDRESS), the device must be assigned an address.

		ADDRESS, in which the device has an address but has not yet been configured. "A USB device responds
		to requests on its default pipe whether the device is currently assigned a unique address or is
		using the default address." Page 242 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		In other words, the device knows its address but an interface has not been configured so it does not
		know how the host will interact with it. To move to the next state the host must issus a SetConfiguration()
		call.

		CONFIGURED, in which the device is active.  In this state the host and device can exchange data.

		SUSPENDED, see page 243 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".  Either the
		device or the host can put the device into suspend mode.  This must be managed by the device driver. On the
		i.MX233 and i.MX6Q, and interrupt is (optinally) issued (see page 5339-5340 of "i.MX 6Dual/6Quad Applications
		Processor Reference Manual Rev. 0, 11/2012").
*/
int usb_state = USB_DEVICE_STATE_DEFAULT;		// unused

/*
	==========================
	Finally the program itself
	==========================
*/
/*
	Pointers to memory (use for allocators)
*/
char *external_memory_head;

/*
	==============================================
	Serial port I/O stuff (for debugging purposes)
	==============================================
*/
#define BAUD_RATE 115200
#define DEFAULT_UART 2		/* can be either 2 (SABRE Lite "console") or 1 (the other UART) */
#define PLL3_FREQUENCY 80000000

/*
	SERIAL_INIT()
	-------------
*/
void serial_init(void)
{
/*
   Enable Clocks
*/
*((uint32_t *)0x020C407C) = 0x0F0000C3;	// CCM Clock Gating Register 5 (CCM_CCGR5) (inc UART clock)

/*
   Enable Pads
*/
#if (DEFAULT_UART == 1)
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	HW_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_MUX_MODE_V(ALT1));
	HW_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_WR(BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_SRE_V(SLOW));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(CSI0_DATA10_ALT3));
	HW_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART1_UART_RX_DATA_SELECT_INPUT_DAISY_V(SD3_DATA6_ALT1));
#elif (DEFAULT_UART == 2)
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA27_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA27_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_SION_V(DISABLED) | BF_IOMUXC_SW_MUX_CTL_PAD_EIM_DATA26_MUX_MODE_V(ALT4));
	HW_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_WR(BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_HYS_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUS_V(100K_OHM_PU) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PUE_V(PULL) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_PKE_V(ENABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_ODE_V(DISABLED) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SPEED_V(100MHZ) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_DSE_V(40_OHM) | BF_IOMUXC_SW_PAD_CTL_PAD_EIM_DATA26_SRE_V(SLOW));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA26_ALT4));
	HW_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_WR(BF_IOMUXC_UART2_UART_RX_DATA_SELECT_INPUT_DAISY_V(EIM_DATA27_ALT4));
#else
	#error "Only UART 1 and 2 are supported"
#endif

/*
	Now on to the UART 8 bits, 1 stop bit, no parity, software flow control
*/
HW_UART_UCR1(DEFAULT_UART).U = 0;
HW_UART_UCR2(DEFAULT_UART).U = 0;
while (HW_UART_UCR2(DEFAULT_UART).B.SRST == 0)
	;	// nothing

HW_UART_UCR1(DEFAULT_UART).B.UARTEN = 1;
HW_UART_UCR3(DEFAULT_UART).B.RXDMUXSEL = 1;
HW_UART_UCR2_WR(DEFAULT_UART, BM_UART_UCR2_WS | BM_UART_UCR2_IRTS | BM_UART_UCR2_RXEN | BM_UART_UCR2_TXEN | BM_UART_UCR2_SRST);
HW_UART_UFCR(DEFAULT_UART).B.RFDIV = 0x04;		/* divide input clock by 2 */
HW_UART_UBIR(DEFAULT_UART).U = 0x0F;
HW_UART_UBMR(DEFAULT_UART).U = (PLL3_FREQUENCY / (HW_CCM_CSCDR1.B.UART_CLK_PODF + 1)) / (2 * BAUD_RATE);		// UBMR should be 0x015B once set
}

/*
	DEBUG_PUTC()
	------------
*/
void debug_putc(char value)
{
HW_UART_UTXD(DEFAULT_UART).U = value;
while (HW_UART_UTS(DEFAULT_UART).B.TXEMPTY == 0)
	; // do nothing
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
	==========================
	General management methods
	==========================
*/

/*
	MAIN_MEMORY_ALLOC()
	-------------------
	Allocate "size" bytes of memory alliged on an "alignment" boundey
*/
void* main_memory_alloc(size_t size, unsigned int alignment)
{
void *result;

/*
	Align the beginning to the requested alignment
*/
uint32_t realignment = alignment - ((uint32_t) external_memory_head % alignment);

if (realignment > 0 && realignment < alignment)
	external_memory_head += realignment;

/*
	Now return the current pointer and move on
*/
result = external_memory_head;
external_memory_head += size;

return result;
}

/*
	===============================================
	i.MX6Q management of endpoints and queue heads
	===============================================
*/

/*
	USB_QUEUE_TD_IN()
	-----------------
	Queue up a transfer IN from the device to the host.  When the transfer has completed and interrupt
	will be issued.  This method converts from endpoint numbers to queue head numbers as queue-heads
	appear to be Freescale specific
*/
void usb_queue_td_in(uint32_t endpoint, const uint8_t *buffer, uint32_t length)
{
uint32_t mask = 0;
imx_endpoint_transfer_descriptor *dTD;
imx_endpoint_queuehead *dQH;

/*
	Set up the transfer descriptor
*/
dTD = &global_dTD[endpoint * 2 + 1];
memset(dTD, 0, sizeof(*dTD));
dTD->next_link_pointer = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_TERMINATOR;
dTD->token.all = 0;
dTD->token.bit.total_bytes = length;
dTD->token.bit.ioc = 1;
dTD->token.bit.status = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_STATUS_ACTIVE;
dTD->buffer_pointer[0] = (char *)buffer;

/*
	Set up the queue head
*/
dQH = &global_queuehead[endpoint * 2 + 1];
dQH->dtd_overlay_area.next_link_pointer = dTD;

/*
	Prime the i.MX USB hardware
*/
mask = BF_USBC_UOG_ENDPTPRIME_PETB(1 << endpoint);
HW_USBC_UOG_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBC_UOG_ENDPTPRIME_RD() & mask) != 0)
	; /* do nothing */
}

/*
	USB_QUEUE_TD_OUT()
	------------------
	Queue up a transfer OUT from the host to the device.  When the message has arrived from the host we
	get an interrupt. This method converts from endpoint numbers to queue head numbers as queueheads
	appear to be Freescale specific
*/
void usb_queue_td_out(uint32_t endpoint)
{
uint32_t mask = 0;
imx_endpoint_transfer_descriptor *dTD;
imx_endpoint_queuehead *dQH;

/*
	Set up the transfer descriptor

	As we can't know how big the transfer is going to be we'll use the same size we allocate
	for the transfer buffer (below).  If the packet is short that'll be OK as the protocol allows
	for short packets. If it is longer the hardware will NAK (I think)
*/
dTD =  &global_dTD[endpoint * 2];
memset(dTD, 0, sizeof(*dTD));
dTD->next_link_pointer = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_TERMINATOR;
dTD->token.all = 0;
dTD->token.bit.total_bytes = IMX_QUEUE_BUFFER_SIZE;
dTD->token.bit.ioc = 1;
dTD->token.bit.status = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_STATUS_ACTIVE;
dTD->buffer_pointer[0] = (char *)global_transfer_buffer[endpoint * 2][0];		// the recieve buffer

/*
	Set up the queue head
*/
dQH = &global_queuehead[endpoint * 2];
dQH->dtd_overlay_area.next_link_pointer = dTD;

/*
	Prime the i.MX USB hardware
*/
mask = BF_USBC_UOG_ENDPTPRIME_PERB(1 << endpoint);
HW_USBC_UOG_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBC_UOG_ENDPTPRIME_RD() & mask) != 0)
	; /* do nothing */
}

/*
	USB_REQUEST_ERROR()
	-------------------
	If the host issues a command and we are unable to respond to it we are supposed to return a STALL.  I doubt this will
	ever happen, but just in case it does.
	See page 247 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
*/
void usb_request_error(uint32_t endpoint)
{
#define HW_USBC_UOG_ENDPTCTRLn_ADDR(n)      (REGS_USBC_BASE + 0x000001C0 + ((n) * 0x4))
#define HW_USBC_UOG_ENDPTCTRLn(n)           (*(volatile hw_usbc_uog_endptctrl0_t *) HW_USBC_UOG_ENDPTCTRLn_ADDR(n))
#define HW_USBC_UOG_ENDPTCTRLn_RD(n)        (HW_USBC_UOG_ENDPTCTRLn(n).U)
#define HW_USBC_UOG_ENDPTCTRLn_WR(n, v)     (HW_USBC_UOG_ENDPTCTRLn(n).U = (v))

HW_USBC_UOG_ENDPTCTRLn_WR(endpoint, HW_USBC_UOG_ENDPTCTRLn_RD(endpoint) & BM_USBC_UOG_ENDPTCTRL0_TXS);
}

/*
	USB_ACK()
	---------
	Send a software ACK to acknowledge the processing of a command

	The harware ACK is taken care of by the i.MX233.
	See table 64-69 page 5349 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	The sofware response ACK is a zero-length packet.
	See page 227 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
*/
void usb_ack(uint32_t endpoint)
{
usb_queue_td_in(endpoint, 0, 0);		// send a 0-length packet
}

/*
	====================
	Generic USB routines
	====================
*/

/*
	PRINT_SETUP_PACKET()
	--------------------
*/
void print_setup_packet(usb_setup_data *packet)
{
debug_print_string("SETUP PACKET\r\n");
debug_print_this("bmRequestType:", packet->bmRequestType.all, "");
debug_print_this("bRequest:", packet->bRequest, "");
debug_print_this("wValue:", packet->wValue, "");
debug_print_this("wIndex:", packet->wIndex, "");
debug_print_this("wLength:", packet->wLength, "");
}

/*
	USB_GET_DESCRIPTOR()
	--------------------
	"The standard request to a device supports three types of descriptors: device (also device_qualifier),
	configuration (also other_speed_configuration), and string. A high-speed capable device supports the
	device_qualifier descriptor to return information about the device for the speed at which it is not operating
	(including wMaxPacketSize for the default endpoint and the number of configurations for the other speed).
	The other_speed_configuration returns information in the same structure as a configuration descriptor, but
	for a configuration if the device were operating at the other speed."
	See page 253-254 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"

	bmRequestType = 0x80
	bRequest = USB_REQUEST_GET_DESCRIPTOR
	wValue = one of the USB_DESCRIPTOR_TYPE_ constants in the high byte. Descriptor index in the low byte (e.g. which string)
	wIndex = 0; or language ID for string descriptors
	wLength = length of the descriptor
	Data = <nothing>

	Valid in states:DEFAULT, ADDRESS, CONFIGURED
*/
void usb_get_descriptor(usb_setup_data *packet)
{
uint32_t descriptor_index;
uint32_t descriptor_type = packet->wValue >> 8;

switch (descriptor_type)
	{
	case USB_DESCRIPTOR_TYPE_DEVICE:
		debug_print_string("USB_DESCRIPTOR_TYPE_DEVICE\r\n");
		/*
			There's some weird hand-shaking going on here.  When we first connect we are required to only send the first 8 bytes
			of the device descriptor.  The host then responds by asking again, we then respond by sending the whole descriptor.
			Now, the first 8 bytes include the length of the descriptor so this appears to be a hand-shaking done so that the
			host can allocate space for the descriptor before asking for it.  If we don't honor this hand-shake then Windows
			appears to consider us to be a badly behaving device and rejects us!


			Although, according to page 39 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000",
			"In order to determine the maximum packet size for the Default Control Pipe, the USB System Software
			reads the device descriptor. The host will read the first eight bytes of the device descriptor. The device
			always responds with at least these initial bytes in a single packet. After the host reads the initial part of the
			device descriptor, it is guaranteed to have read this default pipe's wMaxPacketSize field (byte 7 of the
			device descriptor). It will then allow the correct size for all subsequent transactions. For all other control
			endpoints, the maximum data payload size is known after configuration so that the USB System Software
			can ensure that no data payload will be sent to the endpoint that is larger than the supported size."

			For more information on the device descriptor see page 261-263 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_device_descriptor, packet->wLength != sizeof(our_device_descriptor) ? 8 : sizeof(our_device_descriptor));
		break;
	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_CONFIGURATION\r\n");
		/*
			This descriptor is shared with USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION, so we set its type before sending

			For more information on the configuration descriptor see page 264-266 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		our_com_descriptor.cd.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_com_descriptor, min(packet->wLength, sizeof(our_com_descriptor)));
		break;

	case USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION\r\n");
		/*
			The OTHER_SPEED_CONFIGURATION request is the same as a configurataion request but allows us to have a device that
			works differently at different speeds.  We ignore this and make the device work the same way regardless of speed.
			To do this we just change the descriptor type.

			For more information on the other speed configuration descriptor see page 266-267 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		our_com_descriptor.cd.bDescriptorType = USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_com_descriptor, min(packet->wLength, sizeof(our_com_descriptor)));
		break;

	case USB_DESCRIPTOR_TYPE_STRING:
		descriptor_index = packet->wValue & 0xFF;

		/*
			For more information on string descriptors see page 273-274 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		switch (descriptor_index)
			{
			case USB_STRING_LANGUAGE:
				debug_print_string("STRING - LANG ID\r\n");
				/*
					Here the host is actaully asking for the languages we support rather than a string
					"String index zero for all languages returns a string descriptor that contains an array
					of two-byte LANGID codes supported by the device"

					See page 273 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
				*/
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_language, min(packet->wLength, our_language.bLength));
				break;
			case USB_STRING_MANUFACTURER:
				debug_print_string("STRING - MANUFACTURER\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_manufacturer, min(packet->wLength, our_manufacturer.bLength));
				break;
			case USB_STRING_PRODUCT:
				debug_print_string("STRING - PRODUCT\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_product, min(packet->wLength, our_product.bLength));
				break;
			case USB_STRING_SERIAL_NUMBER:
				debug_print_string("STRING - SERIAL NUMBER\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_serial_number, min(packet->wLength, our_serial_number.bLength));
				break;
			case MS_USB_STRING_OS_DESCRIPTOR:
				debug_print_string("MS OS STRING DESCRIPTOR\r\n");
				/*
					Microsoft OS String Desciptor
				*/
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_ms_os_string_descriptor, sizeof(our_ms_os_string_descriptor));
				break;
			default:
				/*
					If the host is behaving correting this cannot happen
				*/
				debug_print_this("Unhandled STRING DESCRIPTOR:" , descriptor_index, "");
				usb_request_error(0);
				break;
			}
		break;
	case USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
		debug_print_string("USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER\r\n");
		/*
			"The device_qualifier descriptor describes information about a high-speed capable device that would
			change if the device were operating at the other speed".
			See page 264 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_device_qualifier, min(packet->wLength, sizeof(our_device_qualifier)));
		break;
	default:
		/*
			If the host is behaving correting this cannot happen
		*/
		debug_print_string("Unrecognised setup packet (illegal descriptor):\r\n");
		print_setup_packet(packet);
		usb_request_error(0);
		break;
	}
}

/*
	MS_USB_GET_DESCRIPTOR()
	-----------------------
	The behaviour of this method is defined in two documents: "Extended Properties OS Feature Descriptor Specification,
	July 13, 2012" and "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012".

	Microsoft supports 3 descriptor types (passed in wIndex), but type 0x01 (Genre) is described in the July 13, 2012 spec as
	"This descriptor is being considered for future versions of Windows, and no specification is currently available" (page 4
	of "Microsoft OS Descriptors Overview, July 13, 2012")

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t ms_usb_get_descriptor(usb_setup_data *packet)
{
switch (packet->wIndex)
	{
	case MS_USB_DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID:
		debug_print_string("MS_USB_DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID\r\n");
		/*
			See: "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012".
		*/
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_ms_compatible_id_feature_descriptor, sizeof(our_ms_compatible_id_feature_descriptor));
		return 1;
	case MS_USB_DESCRIPTOR_TYPE_EXTENDED_PROPERTIES:
		debug_print_string("MS_USB_DESCRIPTOR_TYPE_EXTENDED_PROPERTIES\r\n");
		/*
			See: Extended Properties OS Feature Descriptor Specification, July 13, 2012"
		*/
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_ms_extended_properties, sizeof(our_ms_extended_properties));
		return 1;
	default:
		debug_print_string("Unrecognised Microsoft descriptor request:\r\n");
		print_setup_packet(packet);
		usb_request_error(0);
		return 1;
	}

return 0;
}

/*
	USB_COMMAND()
	-------------
	The methods we must implement are descibed on page 250 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
	All of them are listed here, even if not implemented (because we can ignore some).

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t usb_command(usb_setup_data *packet)
{
uint8_t *what_to_send;

switch (packet->bRequest)
	{
	case USB_REQUEST_GET_STATUS:
		debug_print_string("USB_REQUEST_GET_STATUS\r\n");
		/*
			See page 254 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = (packet->bmRequestType.bit.recipient == USB_SETUP_RECIPIENT_DEVICE) ? (uint8_t *)&our_one_16 : (uint8_t *)&our_zero_16;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, what_to_send, sizeof(uint16_t));
		return 1;

	case USB_REQUEST_CLEAR_FEATURE:
		debug_print_string("USB_REQUEST_CLEAR_FEATURE\r\n");
		/*
			See page 252 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_ack(0);
		return 1;

	case USB_REQUEST_SET_FEATURE:
		debug_print_string("USB_REQUEST_SET_FEATURE\r\n");
		/*
			See page 258 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_ack(0);
		return 1;

	case USB_REQUEST_SET_ADDRESS:
		debug_print_this("USB_REQUEST_SET_ADDRESS(", packet->wValue, ")");
		/*
			See page 256 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		HW_USBC_UOG_DEVICEADDR_WR(BF_USBC_UOG_DEVICEADDR_USBADR(packet->wValue) | BM_USBC_UOG_DEVICEADDR_USBADRA);
		usb_ack(0);

		usb_setup_endpoint_nonzero();
		return 1;

	case USB_REQUEST_GET_DESCRIPTOR:
		debug_print_string("USB_REQUEST_GET_DESCRIPTOR: ");
		/*
			See page 253 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_get_descriptor(packet);
		return 1;

	case USB_REQUEST_SET_DESCRIPTOR:
		debug_print_string("USB_REQUEST_SET_DESCRIPTOR\r\n");
		/*
			See page 257 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_ack(0);
		return 1;

	case USB_REQUEST_GET_CONFIGURATION:
		debug_print_string("USB_REQUEST_GET_CONFIGURATION\r\n");
		/*
			See page 253 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = &our_one_8;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, what_to_send, sizeof(uint8_t));
		return 1;

	case USB_REQUEST_SET_CONFIGURATION:
		debug_print_string("USB_REQUEST_SET_CONFIGURATION\r\n");
		/*
			See page 257 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_ack(0);
		return 1;

	case USB_REQUEST_GET_INTERFACE:
		debug_print_string("USB_REQUEST_GET_INTERFACE\r\n");
		/*
			See page 254 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = &our_zero_8;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, what_to_send, sizeof(uint8_t));
		return 1;

	case USB_REQUEST_SET_INTERFACE:
		debug_print_string("USB_REQUEST_SET_INTERFACE\r\n");
		/*
			See page 259 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_request_error(0);
		return 1;

	case USB_REQUEST_SYNCH_FRAME:
		debug_print_string("USB_REQUEST_SYNCH_FRAME\r\n");
		/*
			See page 260 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_request_error(0);
		return 1;
	}

return 0;
}

/*
	USB_CDC_COMMAND()
	-----------------
	Manage the CDC commands (of which there aren't many).
	Here we manage only those requests necessary for a CDC virtual COM port

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t usb_cdc_command(usb_setup_data *packet)
{
switch (packet->bRequest)
	{
	case USB_CDC_REQUEST_SET_LINE_CODING:
		debug_print_string("USB_CDC_REQUEST_SET_LINE_CODING\r\n");
		/*
			See page 57 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		usb_ack(0);
		return 1;

	case USB_CDC_REQUEST_GET_LINE_CODING:
		debug_print_string("USB_CDC_REQUEST_GET_LINE_CODING\r\n");
		/*
			See page 58 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (uint8_t *)&our_cdc_line_coding, sizeof(our_cdc_line_coding));
		return 1;

	case USB_CDC_REQUEST_SET_CONTROL_LINE_STATE:
		debug_print_string("USB_CDC_REQUEST_SET_CONTROL_LINE_STATE\r\n");
		/*
			See page 58 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		usb_ack(0);
		return 1;

	case USB_CDC_REQUEST_SEND_BREAK:
		debug_print_string("USB_CDC_REQUEST_SEND_BREAK\r\n");
		/*
			See page 59 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		usb_ack(0);
		return 1;
	}

return 0;
}

/*
	USB_INTERRUPT()
	---------------
*/
void usb_interrupt(void)
{
uint32_t endpoint;
uint32_t endpoint_setup_status;
uint32_t endpoint_status;
uint32_t endpoint_complete;
usb_setup_data setup_packet;

/*
	Grab the value of the various status registers as their values can change while this routine is running
*/
endpoint_setup_status = HW_USBC_UOG_ENDPTSETUPSTAT_RD();
endpoint_status = HW_USBC_UOG_ENDPTSTAT_RD();
endpoint_complete = HW_USBC_UOG_ENDPTCOMPLETE_RD();

/*
	Tell the i.MX that we've serviced those requests
*/
HW_USBC_UOG_ENDPTSETUPSTAT_WR(endpoint_setup_status);
HW_USBC_UOG_ENDPTCOMPLETE_WR(endpoint_complete);
HW_USBC_UOG_ENDPTSTAT.U = endpoint_status;

/*
	The i.MX series has special handeling of a setup message - it puts it directly into the
	transfer descriptor rather than the transfer buffer.  It also sets an internal flag saying
	which endpoint just got the setup message and then generates an interupt.  We check for
	that flag here.
*/
if (endpoint_setup_status & 0x1F)
	{
	/*
		In the CDC model setup packets can only occur on Endpoint 1. If we get a setup packet
		anywhere else then its an error.
	*/
	if (endpoint_setup_status & 0x01)
		{
		/*
			The way we handle a setup event is explicity stated on page 5347-5348 of "i.MX 6Dual/6Quad
			Applications Processor Reference Manual Rev. 0, 11/2012"  where it states:

			"* Disable Setup Lockout by writing 1 to Setup Lockout Mode (SLOM) in USB Device
			   Mode (USBC_n_USBMODE). (once at initialization). Setup lockout is not necessary
			   when using the tripwire as described below."

			That is done when we enable the USB sub-system, but here we must:


			"* After receiving an interrupt and inspecting Endpoint Setup Status
			   (USBC_n_ENDPTSETUPSTAT) to determine that a setup packet was received on a
			   particular pipe:
			   a. Write 1 to clear corresponding bit Endpoint Setup Status
			      (USBC_n_ENDPTSETUPSTAT).
			   b. Write 1 to Setup Tripwire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register.
			   c. Duplicate contents of dQH.SetupBuffer into local software byte array.
			   d. Read Setup TripWire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register. (if set - continue; if cleared - goto 2)
			   e. Write 0 to clear Setup Tripwire (SUTW) in USB Command Register
			      (USBC_n_USBCMD) register.
			   f. Process setup packet using local software byte array copy and execute status/
			      handshake phases."

			OK, so point "d" should read "goto b" not "goto 2" (this is also wrong in the i.MX53 manual).  But
			otherwise, this is what we do here.
		*/

		HW_USBC_UOG_ENDPTSETUPSTAT_WR(1);										// bullet-point "a": clear status register
		do
			{
			HW_USBC_UOG_USBCMD.B.SUTW = 1;										// bullet-point "b": set tripwire
			setup_packet = global_queuehead[0].setup_buffer;					// bullet-point "c": copy packet
			}
		while (HW_USBC_UOG_USBCMD.B.SUTW == 0);									// bullet-point "d": check tripwire
		HW_USBC_UOG_USBCMD.B.SUTW = 0;											// bullet-point "e": clear tripwire

		/*
			bullet-point "f": process the packet

			At this point we have a copy of the setup packet so we can determine from the bmRequestType field
			who the message was destined for...  See page 248 of "Universal Serial Bus Specification Revision
			2.0 April 27, 2000" where it states of bmRequestType:

			"D7: Data transfer direction
				0 = Host-to-device
				1 = Device-to-host
			 D6...5: Type
				0 = Standard
				1 = Class
				2 = Vendor
				3 = Reserved
			 D4...0: Recipient
				0 = Device
				1 = Interface
				2 = Endpoint
				3 = Other
				4...31 = Reserved"

			The messages we expect are:
			80, 00 (1000 0000b and 0000 0000b) Standard, Device: USB
			A1, 21 (1010 0001b and 0010 0001b) Class, Interface: USB CDC
			C0, C1 (1100 0000b and 1100 0000b) Vendor, Device  : Microsoft Extensions
		*/
		if (setup_packet.bmRequestType.bit.type == USB_SETUP_TYPE_STANDARD)
			usb_command(&setup_packet);
		else if (setup_packet.bmRequestType.bit.type == USB_SETUP_TYPE_CLASS)
			usb_cdc_command(&setup_packet);
		else if (setup_packet.bmRequestType.bit.type == USB_SETUP_TYPE_VENDOR)
			ms_usb_get_descriptor(&setup_packet);
		}
	else
		debug_print_this("SETUP PACKET recieved on an endpoint other than endpoint 0 (HW_USBC_UOG_ENDPTSETUPSTAT = ", HW_USBC_UOG_ENDPTSETUPSTAT_RD(), ")");
	}

/*
	Manage any send/recieve completion events and any notifications that the endpoints are no longer primed
*/
for (endpoint = 0; endpoint < IMX_USB_MAX_ENDPOINTS; endpoint++)
	{
	/*
		Recieve on and endpoint has completed
	*/
	if (endpoint_complete & BF_USBC_UOG_ENDPTCOMPLETE_ERCE(1 << endpoint))
		if (endpoint != 0)
			{
			/*
				If we're on the data endpoint then echo back and queue up a request to recieve data
			*/
			usb_queue_td_in(endpoint, (uint8_t *)&((global_transfer_buffer[endpoint * 2][0])[0]), 1);
			usb_queue_td_out(endpoint);
			}

	/*
		If we're on endpoint 0 then we make sure we are always ready to receive
	*/
#define BF_USBC_UOG_ENDPTSTAT_ERBR(v)   (((v) << 0) & BM_USBC_UOG_ENDPTSTAT_ERBR)
	if (endpoint == 0 && (!(endpoint_status & BF_USBC_UOG_ENDPTSTAT_ERBR(1 << endpoint))))
		usb_queue_td_out(endpoint);

	/*
		Transmit on an endpoint has completed
	*/
	if (endpoint_complete & BF_USBC_UOG_ENDPTCOMPLETE_ETCE(1 << endpoint))
		/* nothing */ ;

	/*
		We are ready to transmit on the endpoint (i.e. queued up)
	*/
#define BF_USBC_UOG_ENDPTSTAT_ETBR(v)   (((v) << 0) & BM_USBC_UOG_ENDPTSTAT_ETBR)
	if (endpoint_status & BF_USBC_UOG_ENDPTSTAT_ETBR(1 << endpoint))
		/* nothing */ ;
	}
}

/*
	ARM_GENERIC_INTERRUPT_CONTROLLER_DISTRIBUTOR_REGISTER_MAP
	---------------------------------------------------------
	See page 4-2 to 4-4 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
	There should be one of these per CPU
*/
typedef struct
{
uint32_t distributor_control_register;							      // 0x0000
uint32_t interrupt_controller_type_register;					      // 0x0004
uint32_t distributor_implementer_identification_register;		// 0x0008
uint32_t reserved1[29];										            // 0x000C
uint32_t interrupt_security_registers[8];						   // 0x0080
uint32_t reserved5[24];
uint32_t interrupt_set_enable_registers[32];					      // 0x0100
uint32_t interrupt_clear_enable_registers[32];					   // 0x0180
uint32_t interrupt_set_pending_registers[32];					   // 0x0200
uint32_t interrupt_clear_pending_registers[32];				      // 0x0280
uint32_t active_bit_registers[32];								      // 0x0300
uint32_t reserved2[32];										            // 0x0380
uint8_t interrupt_priority_registers[255 * sizeof(uint32_t)];					      // 0x0400
uint32_t reserved3;															// 0x07FC
uint8_t interrupt_processor_targets_registers[255 * sizeof(uint32_t)];					// 0x0800
uint32_t reserved4;															// 0x0BFC
uint32_t interrupt_configuration_registers[64];						// 0x0C00
uint32_t implementation_defined_registers[64];						// 0x0D00
uint32_t reserved6[64];														// 0x0E00
uint32_t software_generated_interrupt_register;						// 0x0F00
uint32_t reserved7[51];														// 0x0F04
uint32_t peripheral_id4;													// 0x0FD0	the next 12 32-bit words are implementation defined identification_registers
uint32_t peripheral_id5;													// 0x0FD4
uint32_t peripheral_id6;													// 0x0FD8
uint32_t peripheral_id7;													// 0x0FDC
uint32_t peripheral_id0;													// 0x0FE0
uint32_t peripheral_id1;													// 0x0FE4
uint32_t peripheral_id2;													// 0x0FE8
uint32_t peripheral_id3;													// 0x0FEC
uint32_t component_id0;														// 0x0FF0
uint32_t component_id1;														// 0x0FF4
uint32_t component_id2;														// 0x0FF8
uint32_t component_id3;														// 0x0FFC
} ARM_generic_interrupt_controller_distributor_register_map;

#define IMX_INT_SPURIOUS 1023

/*
	ARM_GENERIC_INTERRUPT_CONTROLLER_CPU_REGISTER_MAP
	-------------------------------------------------
	See page 4-4 to 4-5 of the "ARM Generic Interrupt Controller Architecture Specification, Architecture version 1.0"
*/
typedef struct
{
uint32_t cpu_interface_control_register;							// 0x00
uint32_t interrupt_priority_mask_register;						// 0x04
uint32_t binary_point_register;										// 0x08
uint32_t interrupt_acknowledge_register;							// 0x0C
uint32_t end_of_interrupt_register;									// 0x10
uint32_t running_priority_register;									// 0x14
uint32_t highest_pending_interrupt_register;						// 0x18
uint32_t aliased_binary_point_register;							// 0x1C
uint32_t reserved1[8];													// 0x20
uint32_t implementation_defined_registers[36];					// 0x40
uint32_t reserved2[11];													// 0xD0
uint32_t cpu_interface_dentification_register;					// 0xFC
} ARM_generic_interrupt_controller_cpu_register_map;

/*
	ISR_IRQ()
	---------
*/
void isr_IRQ(void) __attribute__((interrupt("IRQ")));
void isr_IRQ(void)
{
uint32_t base;
volatile ARM_generic_interrupt_controller_cpu_register_map *cpu_registers;
volatile uint32_t got = 0;
hw_usbc_uog_usbsts_t usb_status;

/*
	Get the address of the CPU's configuration registers, but in the address space of the SOC.
	This instruction only works on the Cortex-A9 MPCore, it does not work on a unicore Cortex A9.
	It has been tested on the i.MX6Q
*/
asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);
cpu_registers = (ARM_generic_interrupt_controller_cpu_register_map *)(base + 0x100);

/*
   ACK the interrupt and tell the hardware that we're in the interrupt service routine
*/
got = cpu_registers->interrupt_acknowledge_register;

/*
   Make sure it wasn't a spurious interrupt
	The Cortex A9 MPCore Reference Manual (page 3-3) makes it clear that no semaphore is
	required to avoid the race condition.  This test for spurious is enough
*/
if (got == IMX_INT_SPURIOUS)
   return;

if (got == IMX_INT_USBOH3_UOTG)
	{
	/*
		Grab the cause of the interrupt and acknowledge it
	*/
	usb_status = HW_USBC_UOG_USBSTS;
	HW_USBC_UOG_USBSTS = usb_status;

	/*
		After initialisation and under normal operation we expect only this case.
	*/
	if (usb_status.B.UI)
		usb_interrupt();

	/*
		USB Reset Interrupt
	*/
	if (usb_status.B.URI)
		{
		debug_print_string("i.MX USB Reset interrupt\r\n");

		/*
			This happens when the host wants to re-initialise the USB bus
			which also happens on startup.

			See page 5338-5339 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
			where it states that on this interrupt we must:

			"Clear all setup token semaphores by reading the Endpoint Status
			 (USBC_n_ENDPTSTAT) register and writing the same value back to the Endpoint
			 Status (USBC_n_ENDPTSTAT) register.

			 Clear all the endpoint complete status bits by reading the Endpoint Complete
			 (USBC_n_ENDPTCOMPLETE) register and writing the same value back to the
			 Endpoint Complete (USBC_n_ENDPTCOMPLETE) register.

			 Cancel all primed status by waiting until all bits in the Endpoint Prime
			 (USBC_n_ENDPTPRIME) are 0 and then writing 0xFFFFFFFF to Endpoint Flush
			 (USBC_n_ENDPTFLUSH).

			 Read the reset bit in the Port Status & Control (USBC_n_PORTSC1) register and make
			 sure that it is still active. A USB reset will occur for a minimum of 3 ms and the DCD
			 must reach this point in the reset cleanup before end of the reset occurs, otherwise a
			 hardware reset of the device controller is recommended (rare.)

				A hardware reset can be performed by writing a one to the device controller reset bit
				in the USBCMD reset. Note: a hardware reset will cause the device to detach from
				the bus by clearing the Run/Stop bit. Thus, the DCD must completely re-initialize the
				device controller after a hardware reset.

			 Free all allocated dTDs because they will no longer be executed by the device controller.
			 If this is the first time the DCD is processing a USB reset event, then it is likely that no
			 dTDs have been allocated.

			 At this time, the DCD may release control back to the OS because no further changes to
			 the device controller are permitted until a Port Change Detect is indicated.
			 After a Port Change Detect, the device has reached the default state and the DCD can
			 read the Port Status & Control (USBC_n_PORTSC1) to determine if the device is
			 operating in FS or HS mode. At this time, the device controller has reached normal
			 operating mode and DCD can begin enumeration according to the USB Chapter 9 -
			 Device Framework."

		*/

		HW_USBC_UOG_ENDPTSTAT.U = HW_USBC_UOG_ENDPTSTAT_RD();			// first point
		HW_USBC_UOG_ENDPTCOMPLETE_WR(HW_USBC_UOG_ENDPTCOMPLETE_RD());	// second point

		while (HW_USBC_UOG_ENDPTPRIME_RD() != 0)						// third point
			 /* do nothing */ ;
		HW_USBC_UOG_ENDPTFLUSH_WR(0xFFFFFFFF);						// third point

		if (HW_USBC_UOG_PORTSC1.B.PR != 1)							// fourth point
			{
			/*
				Hard reset needed
			*/
			}
		// we can ignore the fifth point because they are already allocated.  We ignore the sixth point as this is normal behaviour

		/*
			We do not need to change our address back to 0 because that automatically done for us
		*/
		}
	/*
		USB Port Change Interrupt
		If we disable this then when the user disconnects and reconnects then we don't get a reset message!!!
	*/
	if (usb_status.B.PCI)
		{
		debug_print_string("i.MX USB port change interrupt\r\n");

		/*
			Page 5319 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
			We get this interrupt when the hardware detects:
				Connect Status Change
				Port Enable/Disable Change
				Over-current Change
				Force Port Resume
		*/
		}
	}

/*
	Tell the interrupt controller that we've finished processing the Interrupt
*/
cpu_registers->end_of_interrupt_register = got;

return;
}

/*
	===================
	Generic ARM methods
	===================
*/
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
	===============================================
	Setup the i.MX System on Chip (SoC) sub-systems
	===============================================
*/

/*
	USB_CONTROLLER_STARTUP()
	------------------------
*/
void usb_controller_startup()
{
/*
	Tell the controller we're a device
*/
HW_USBC_UOG_USBMODE.B.CM = 2;			// DEVICE MODE

/*
	Clear all the status bits
*/
HW_USBC_UOG_ENDPTSETUPSTAT = HW_USBC_UOG_ENDPTSETUPSTAT;
HW_USBC_UOG_ENDPTCOMPLETE = HW_USBC_UOG_ENDPTCOMPLETE;
HW_USBC_UOG_ENDPTSTAT.U = HW_USBC_UOG_ENDPTSTAT.U;
HW_USBC_UOG_USBSTS_SET
	(
	BM_USBC_UOG_USBSTS_URI | // USB USB Reset Received
	BM_USBC_UOG_USBSTS_PCI | // USB Port Change Detect
	BM_USBC_UOG_USBSTS_UI	// USB Interrupt (USBINT)
	);

/*
	Enable interrupts
	The Port Change Detect interrupt is essential for a disconnect and reconnect while we're powered up.  If
	we don't enable the port connect then we don't get the reset interrupt to tell us we've just connected to
	a host.
*/
HW_USBC_UOG_USBINTR_WR(
	BM_USBC_UOG_USBINTR_URE | 	// USB Reset Enable
	BM_USBC_UOG_USBINTR_PCE |	// USB Port Change Detect
	BM_USBC_UOG_USBINTR_UE 	 	// USB Interrupt Enable.
	);

/*
	Set the port speed, options are:
		BV_USBC_UOG_PORTSC1_PSPD__FULL  0		// 12Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__LOW   1		// 1.5Mb/s
		BV_USBC_UOG_PORTSC1_PSPD__HIGH  2		// 480Mb/s
*/
HW_USBC_UOG_PORTSC1.B.PSPD = 2;

/*
	Turn on the trip-wire mechanism for Setup packets
*/
HW_USBC_UOG_USBMODE.B.SLOM = 1;

/*
	now start the USB sub-system
*/
HW_USBC_UOG_USBCMD.B.RS = 1;
}

/*
	==================================================
	Program specific set up of the i.MX USB sub-system
	==================================================
*/
/*
	CONFIG_ENDPOINT()
	-----------------
*/
void config_endpoint(int which)
{
imx_endpoint_transfer_descriptor *td;

/*
	Get the correct transfer descriptor
*/
td = &global_dTD[which];

/*
	Terminate the linked list
*/
td->next_link_pointer = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_TERMINATOR;

/*
	Set up the queuehead
*/
global_queuehead[which].dtd_overlay_area.next_link_pointer = (imx_endpoint_transfer_descriptor *)(((uint32_t)td) + 1);			// end of chain
global_queuehead[which].dtd_overlay_area.token.all = 0;
global_queuehead[which].dtd_overlay_area.token.bit.ioc = 1;
global_queuehead[which].capabilities.all = 0;
global_queuehead[which].capabilities.bit.maximum_packet_length = USB_MAX_PACKET_SIZE;
global_queuehead[which].capabilities.bit.ios = 1;

/*
	Allocate a transfer buffer and put that in the queuehead too
*/
if (global_transfer_buffers_preallocated == 0)
	global_transfer_buffer[which][0] = (uint8_t *)main_memory_alloc(IMX_QUEUE_BUFFER_SIZE, IMX_QUEUE_BUFFER_SIZE);
global_queuehead[which].dtd_overlay_area.buffer_pointer[0] = (void *)global_transfer_buffer[which][0];
}

/*
	USB_SETUP_ENDPOINT_ZERO()
	-------------------------
*/
void usb_setup_endpoint_zero()
{
/*
	Zero everything (queheads then transder descriptors)
*/
memset(global_queuehead, 0, sizeof (global_queuehead));
memset(global_dTD, 0, sizeof (global_dTD));

/*
	Queue 0 and 1 (Endpoint 0): USB Control
*/
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_IN);

/*
	Hand off to the i.MX233
*/
HW_USBC_UOG_ENDPTLISTADDR_WR((uint32_t)global_queuehead);

/*
	Start listening
*/
usb_queue_td_out(0);
}

/*
	USB_SETUP_ENDPOINTS_NONZERO()
	-----------------------------
*/
void usb_setup_endpoint_nonzero()
{
hw_usbc_uog_endptctrl1_t endpointcfg;

#define IMX_USB_ENDPTCTRL_CONTROL      0
#define IMX_USB_ENDPTCTRL_ISOCHRONOUS  1
#define IMX_USB_ENDPTCTRL_BULK         2
#define IMX_USB_ENDPTCTRL_INTERRUPT    3

/*
	Queue 2 and 3 (Endpoint 1): Abstract Control Management Interface
*/
endpointcfg.U = 0;										// Initialise
endpointcfg.B.TXE = 1; 								// Transmit enable
endpointcfg.B.TXR = 1; 								// Reset the PID
endpointcfg.B.TXT = IMX_USB_ENDPTCTRL_INTERRUPT;		// We're a USB "interrupt" endpoint
endpointcfg.B.RXE = 1;									// Recieve enable
endpointcfg.B.RXR = 1;									// Reset the PID
endpointcfg.B.RXT = IMX_USB_ENDPTCTRL_INTERRUPT;		// We're a USB "interrupt" endpoint

/*
	Allocate transfer buffers and enable the endpoint
*/
config_endpoint(IMX_USB_CDC_QUEUEHEAD_ACM_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_ACM_IN);
HW_USBC_UOG_ENDPTCTRLn_WR(USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT, endpointcfg.U);		// Endpoint 1

/*
	Queue 4 and 5 (Endpoint 2): Data from host
*/
endpointcfg.U = 0;										// Initialise
endpointcfg.B.TXE = 1; 								// Transmit enable
endpointcfg.B.TXR = 1; 								// Reset the PID
endpointcfg.B.TXT = IMX_USB_ENDPTCTRL_BULK;			// We're a USB "bulk" endpoint
endpointcfg.B.RXE = 1;									// Recieve enable
endpointcfg.B.RXR = 1;									// Reset the PID
endpointcfg.B.RXT = IMX_USB_ENDPTCTRL_BULK;			// We're a USB "bulk" endpoint

/*
	Allocate transfer buffers and enable the endpoint
*/
config_endpoint(IMX_USB_CDC_QUEUEHEAD_DATA_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_DATA_IN);
HW_USBC_UOG_ENDPTCTRLn_WR(USB_CDC_ENDPOINT_SERIAL, endpointcfg.U);		// EP 2

/*
	Update the i.MX233's pointer to the queueheads (actually, not necessary as the address hasn't changed)
*/
HW_USBC_UOG_ENDPTLISTADDR_WR((uint32_t)global_queuehead);

/*
	Prime the endpoints
*/
usb_queue_td_out(USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT);
usb_queue_td_out(USB_CDC_ENDPOINT_SERIAL);

/*
	Make a note of the fact that we have already allocated the buffers once
	this is done to prevent re-allocation on disconnect and reconnect
*/
global_transfer_buffers_preallocated = 1;
}

/*
	==========================
	i.MX6Q interrupt handeling
	==========================
*/

/*
	ARM_INTERRUPT_VECTORS
	---------------------
*/
typedef struct
{
uint32_t reset;
uint32_t undefined_instruction;
uint32_t swi;
uint32_t prefetch_abort;
uint32_t data_abort;
uint32_t reserved;
uint32_t irq;
uint32_t firq;
uint32_t sw_monitor;
} ARM_interrupt_vectors;

/*
	CPU_INTERRUPT_INIT()
	--------------------
*/
void cpu_interrupt_init(void)
{
ARM_interrupt_vectors *vectors = (ARM_interrupt_vectors *)0x0093FFDC;

vectors->irq = (uint32_t)isr_IRQ;
enable_IRQ();
}

/*
   INTERRUPT_INIT()
   ----------------
*/
void interrupt_init(void)
{
uint32_t base;
volatile ARM_generic_interrupt_controller_cpu_register_map *cpu_registers;
volatile ARM_generic_interrupt_controller_distributor_register_map *distributor_registers;

asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);
cpu_registers = (ARM_generic_interrupt_controller_cpu_register_map *)(base + 0x100);
distributor_registers = (ARM_generic_interrupt_controller_distributor_register_map *)(base + 0x1000);

cpu_registers->interrupt_priority_mask_register = 0xFF;
cpu_registers->cpu_interface_control_register = 0x03;
distributor_registers->distributor_control_register = 0x03;
}

/*
	GRAB_INTERRUPT()
	----------------
*/
void grab_interrupt(uint32_t which)
{
uint32_t base;
volatile ARM_generic_interrupt_controller_distributor_register_map *distributor_registers;

asm volatile
	(
	"MRC p15, 4, %0, c15, c0, 0;"
	: "=r"(base)
	:
	:
	);
distributor_registers = (ARM_generic_interrupt_controller_distributor_register_map *)(base + 0x1000);

distributor_registers->interrupt_priority_registers[which] = 0;
distributor_registers->interrupt_security_registers[which / 32] &= ~(1 << (which & 0x1F));
distributor_registers->interrupt_processor_targets_registers[which] |= 1;
distributor_registers->interrupt_set_enable_registers[which / 32] = 1 << (which & 0x1F);
}

/*
	==========================================
	u-Second timer code (using the i.MX6Q EPTI
	==========================================
*/
#define DEFAULT_TIMER 1
/*
	DELAY_INIT()
	------------
*/
void delay_init(void)
{
uint32_t speed_in_Hz[] = {528000000, 396000000, 352000000, 198000000, 594000000};
uint32_t frequency;

HW_CCM_CCGR1.B.CG6 = 0x03;

HW_EPIT_CR_WR(DEFAULT_TIMER, BM_EPIT_CR_SWR);
while ((HW_EPIT_CR(DEFAULT_TIMER).B.SWR) != 0)
	;	// nothing

frequency = speed_in_Hz[HW_CCM_CBCMR.B.PRE_PERIPH_CLK_SEL] / (HW_CCM_CBCDR.B.AHB_PODF + 1) / (HW_CCM_CBCDR.B.IPG_PODF + 1);
HW_EPIT_CR_WR(DEFAULT_TIMER, BF_EPIT_CR_CLKSRC(1) | BF_EPIT_CR_PRESCALAR((frequency / 1000000) - 1) | BM_EPIT_CR_RLD | BM_EPIT_CR_IOVW | BM_EPIT_CR_ENMOD);
}

/*
	DELAY_US()
	----------
*/
void delay_us(uint32_t time_in_us)
{
HW_EPIT_LR_WR(DEFAULT_TIMER, time_in_us);
HW_EPIT_SR_SET(DEFAULT_TIMER, BM_EPIT_SR_OCIF);
HW_EPIT_CR_SET(DEFAULT_TIMER, BM_EPIT_CR_EN);

while (HW_EPIT_SR_RD(DEFAULT_TIMER) == 0)
	;	// nothing (i.e. wait)

HW_EPIT_CR_CLR(DEFAULT_TIMER, BM_EPIT_CR_EN);
}

/*
	======
	MAIN()
	======
*/
int main(void)
{
uint32_t irq_stack[256];
uint32_t *irq_sp = irq_stack + sizeof(irq_stack);

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

external_memory_head = (char *) &ATOSE_start_of_heap;

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
	Set up the serial ports
*/
serial_init();
delay_init();

debug_print_string("Disconnect the USB Phy...");
HW_USBPHY_CTRL(1).B.CLKGATE = 1;

debug_print_string("Wait...");
delay_us(1000000);			// 1 second delay
debug_print_string("Reconnect the USB Phy");
HW_USBPHY_CTRL(1).B.CLKGATE = 0;
while (HW_USBPHY_CTRL(1).B.CLKGATE)
	/* do nothing */ ;


/*
	Set up the interrupt controller, the CPU's interrupt vectors, and grab the USB interrupt
*/
interrupt_init();
cpu_interrupt_init();
grab_interrupt(IMX_INT_USBOH3_UOTG);

/*
	Now we're up and running...
*/
debug_print_string("\r\ni.MX6Q USB CDC Loopback\r\nby Andrew Trotman and Nick Sherlock\r\nCopyright (c) 2013\r\n\r\n");

/*
	Start the USB Controller
*/
debug_print_string("USB controller startup... ");
global_transfer_buffers_preallocated = 0;
usb_controller_startup();
usb_setup_endpoint_zero();
debug_print_string(" done!\r\n");

for (;;);				// loop forever

return 0;
}

