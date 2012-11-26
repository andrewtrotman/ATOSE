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
	States that the USB device harware can be in
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
#define USB_REQUEST_GET_STATUS 			0x00
#define USB_REQUEST_CLEAR_FEATURE 			0x01
//#define USB_REQUEST_RESERVED				0x02
#define USB_REQUEST_SET_FEATURE 			0x03
//#define USB_REQUEST_RESERVED				0x04
#define USB_REQUEST_SET_ADDRESS 			0x05
#define USB_REQUEST_GET_DESCRIPTOR 		0x06		// Implemented
#define USB_REQUEST_SET_DESCRIPTOR 		0x07
#define USB_REQUEST_GET_CONFIGURATION 		0x08
#define USB_REQUEST_SET_CONFIGURATION 		0x09
#define USB_REQUEST_GET_INTERFACE 			0x0A
#define USB_REQUEST_SET_INTERFACE 			0x0B
#define USB_REQUEST_SYNCH_FRAME 			0x0C

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
#ifdef IMX233
	/*
		The i.MX233 allows 5 endpoints 0..4 (see page 8-1 of "i.MX23 Applications Processor Reference Manual Rev. 1 11/2009"
	*/
	#define IMX_USB_MAX_ENDPOINTS 5
#else
	/*
		The i.MX53 and i.MX6Q have 8 endpoints 0..7
			see page 5449 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
			see page 4956 of "i.MX53 Multimedia Applications Processor Reference Manual Rev. 2.1, 06/2012"
	*/
	#define IMX_USB_MAX_ENDPOINTS 8
#endif


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










#define USB_DIRECTION_OUT						0x00
#define USB_DIRECTION_IN						0x01




#define USB_CDC_SET_LINE_CODING				0x20
#define USB_CDC_GET_LINE_CODING				0x21
#define USB_CDC_SET_CONTROL_LINE_STATE			0x22

#define USB_REQUEST_MS_GET_EXTENDED_PROPERTIES		0x05



void usb_setup_endpoint_nonzero(void);
void usb_setup_endpoint_zero(void);


/*
	=========
	USB stuff
	=========
*/
/*
	struct USB_SETUP_DATA
	---------------------
	Page 248 of "Universal Serial Bus Specification Revision 2.0"
	This structure is the shape of the setup packet sent from the host to the device.
	It is used in communications before the device comes on line
*/
typedef struct
{
uint8_t bmRequestType;
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
	i.MX233 stuff
	=============
	The i.MX233 data sheet does not discuss these structures at all, they are assumed.  Freescale discuss them
	in various documents, but since this stuff is the same on the i.MX6Q, the structures are taken from there.
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
	
	The Endpoint Transfer Descriptor data struct must be 32-bye alligned (and is 32-bytes long) hence the padding at the end
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
.bLength = sizeof(our_serial_number),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'S', 0x00, 'N', 0x00, '#', 0x00, 'D', 0x00, 'E', 0x00, 'V', 0x00}
} ;

/*
	OUR_MANUFACTURER
	----------------
*/
usb_string_descriptor our_manufacturer =
{
.bLength = sizeof(our_manufacturer),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'A', 0x00, 'S', 0x00, 'P', 0x00, 'T', 0x00}
} ;

/*
	OUR_PRODUCT
	-----------
*/
usb_string_descriptor our_product =
{
.bLength = sizeof(our_product),
.bDescriptorType = USB_DESCRIPTOR_TYPE_STRING,
.wString = {'F', 0x00, 'o', 0x00, 'u', 0x00, 'r', 0x00, 'A', 0x00, 'R', 0x00, 'M', 0x00}
} ;


/*

	I based this on the USB Serial Example for Teensy USB Development
	Board (http://www.pjrc.com/teensy/usb_serial.html) but as its almost
	completely re-written and isn't "substantial portions of the Software" 
	and that software is under a BSD-like licence, I don't reproduce their 
	copyright notice here.

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
	Now on to the i.MX233
	=====================
*/

/*
	There are two queue heads for each endpoint, even numbers are OUT going from host, odd numbers are IN coming to host
	"The Endpoint Queue Head List must be aligned to a 2k boundary"  see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	On the i.MX233 the queue heads must be in on-chip RAM (but apparently not on the i.MX6Q)
		"The i.MX23 has the bandwidth to handle the data buffers in DRAM for both high-speed and full-speed
		USB transmissions. However, the queue heads (dQH) must be placed in on-chip RAM. A design limitation
		on burst size does not allow the queue heads to be placed in DRAM."
		see page 8-3 of "i.MX23 Applications Processor Reference Manual Rev. 1 11/2009"
*/
imx_endpoint_queuehead global_queuehead[IMX_USB_MAX_ENDPOINTS * 2] __attribute__ ((aligned (2048)));

/*
	Each endpoint has a transfer descriptor
	Endpoint transfer descriptors must be aligned on 32-byte boundaries because the last 5 bits must be zero
	Also note that the last bit is a termination bit (T), see page 5329 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"
*/
imx_endpoint_transfer_descriptor global_dTD[IMX_USB_MAX_ENDPOINTS * 2] __attribute__ ((aligned (32)));

/*
	Each transfer descripter has a pointer to 5 seperate transfer buffers. We're only going to use one of these but we'll
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
int usb_state = USB_DEVICE_STATE_DEFAULT;

/*
	==========================
	Finally the program itself
	==========================
*/
/*
	Pointers to memory (use for allocators)
*/
char *external_memory_head = (char *) 0x40000000;
char *internal_memory_head;

/*
	==============================================
	Serial port I/O stuff (for debugging purposes)
	==============================================
*/
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
	DELAY_US()
	----------
	wait "us" microseconds.
*/
void delay_us(unsigned long int us)
{
unsigned long int start = HW_DIGCTL_MICROSECONDS_RD();
unsigned long int end = start + us;

while ((int32_t) (HW_DIGCTL_MICROSECONDS_RD() - end) < 0)
	;/* nothing */
}

/*
	===============================================
	i.MX233 management of endpoints and queue heads
	===============================================
*/

/*
	USB_QUEUE_TD_IN()
	-----------------
	Queue up a transfer IN from the device to the host.  When the transfer has completed and interrupt
	will be issued.  This method converts from endpoint numbers to queue head numbers as queueheads
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
mask = BF_USBCTRL_ENDPTPRIME_PETB(1 << endpoint);
HW_USBCTRL_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBCTRL_ENDPTPRIME_RD() & mask) != 0)
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
mask = BF_USBCTRL_ENDPTPRIME_PERB(1 << endpoint);
HW_USBCTRL_ENDPTPRIME_SET(mask);

/*
	Wait until the prime has completed
*/
while ((HW_USBCTRL_ENDPTPRIME_RD() & mask) != 0)
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
HW_USBCTRL_ENDPTCTRLn_WR(endpoint, HW_USBCTRL_ENDPTCTRLn_RD(endpoint) & BM_USBCTRL_ENDPTCTRLn_TXS);
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
debug_print_this("bmRequestType:", packet->bmRequestType, "");
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
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char*)&our_device_descriptor, packet->wLength != sizeof(our_device_descriptor) ? 8 : sizeof(our_device_descriptor));
		usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
		break;
	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_CONFIGURATION\r\n");
		/*
			This descriptor is shared with USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION, so we set its type before sending

			For more information on the configuration descriptor see page 264-266 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		our_com_descriptor.cd.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION;
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_com_descriptor, sizeof(our_com_descriptor));
		usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
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
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_com_descriptor, sizeof(our_com_descriptor));
		usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
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
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_language, sizeof(our_language));
				usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
				break;
			case USB_STRING_MANUFACTURER:
				debug_print_string("STRING - MANUFACTURER\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_manufacturer, sizeof(our_manufacturer));
				usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
				break;
			case USB_STRING_PRODUCT:
				debug_print_string("STRING - PRODUCT\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_product, sizeof(our_product));
				usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
				break;
			case USB_STRING_SERIAL_NUMBER:
				debug_print_string("STRING - SERIAL NUMBER\r\n");
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_serial_number, sizeof(our_serial_number));
				usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
				break;
			case MS_USB_STRING_OS_DESCRIPTOR:
				debug_print_string("MS OS STRING DESCRIPTOR\r\n");
				/*
					Microsoft OS String Desciptor
				*/
				usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_ms_os_string_descriptor, sizeof(our_ms_os_string_descriptor));
				usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
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
		usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_device_qualifier, sizeof(our_device_qualifier));
		usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
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
	The behaviour of this method is defined in two documents: "Extended Properties OS Feature Descriptor Specification, July 13, 2012"
	and "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012". 
*/
void ms_usb_get_descriptor(usb_setup_data *packet)
{
if (packet->bmRequestType == MS_USB_REQUEST_GET_EXTENDED_COMPAT_ID_OS_FEATURE_DESCRIPTOR)
	{
	debug_print_string("MS_USB_REQUEST_GET_EXTENDED_COMPAT_ID_OS_FEATURE_DESCRIPTOR\r\n");
	/*
		See: "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012". 
	*/
	usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_ms_compatible_id_feature_descriptor, sizeof(our_ms_compatible_id_feature_descriptor));
	usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
	}
else if (packet->bmRequestType == MS_USB_REQUEST_GET_EXTENDED_PROPERTIES_OS_DESCRIPTOR)
	{
	switch (packet->wIndex)
		{
		case USB_REQUEST_MS_GET_EXTENDED_PROPERTIES:
			debug_print_string("USB_REQUEST_MS_GET_EXTENDED_PROPERTIES\r\n");
			/*
				See: Extended Properties OS Feature Descriptor Specification, July 13, 2012"
			*/
			usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_ms_extended_properties, sizeof(our_ms_extended_properties));
			usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
			break;
		default:
			debug_print_string("Unrecognised Microsoft descriptor request:\r\n");
			print_setup_packet(packet);
			usb_request_error(0);
			break;
		}
	}
}


/*
	USB_INTERRUPT()
	---------------
*/
void usb_interrupt(void)
{
int handled = 0;
uint16_t new_address;

//Do we have a pending endpoint setup event to handle?
if (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 0x1F)
	{
	if (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 1)
		{
//		debug_print_string("USB SETUP request.\r\n");
	
		usb_setup_data setup_packet;

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
			setup_packet = global_queuehead[USB_DIRECTION_OUT].setup_buffer;
			}
		while (HW_USBCTRL_USBCMD.B.SUTW == 0); //Keep looping until we succeed

		HW_USBCTRL_USBCMD.B.SUTW = 0;

		// Clear the bit in the setup status register by writing a 1 there:
		do
			HW_USBCTRL_ENDPTSETUPSTAT_WR(1);
		while (HW_USBCTRL_ENDPTSETUPSTAT_RD() & 1);


		if (setup_packet.bmRequestType & 0x80)
			{
			//Packets with a device-to-host data phase direction

			
			if (setup_packet.bmRequestType == MS_USB_REQUEST_GET_EXTENDED_COMPAT_ID_OS_FEATURE_DESCRIPTOR || setup_packet.bmRequestType == MS_USB_REQUEST_GET_EXTENDED_PROPERTIES_OS_DESCRIPTOR)
				{
				/*
					Manage the Microsoft Extensions
				*/
				ms_usb_get_descriptor(&setup_packet);
				}
			else if (setup_packet.bmRequestType == 0xA1)
				{
				/*
					Manage the CDC commands
				*/
				switch (setup_packet.bRequest)
					{
					case USB_CDC_GET_LINE_CODING:
						debug_print_string("USB_CDC_GET_LINE_CODING\r\n");
						usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, (char *)&our_cdc_line_coding, sizeof(our_cdc_line_coding));
						usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
						break;
					default:
						print_setup_packet(&setup_packet);
						debug_print_this("Unhandled device-to-host data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			else
				{
				/*
					Manage device commands
				*/
				switch (setup_packet.bRequest)
					{
					case USB_REQUEST_GET_DESCRIPTOR:
						usb_get_descriptor(&setup_packet);
						handled = 1;
						break;
					default:
						print_setup_packet(&setup_packet);
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
						usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, 0, 0);				// respond with a 0-byte transfer (ACK)
						usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);
						break;
					case USB_CDC_SET_LINE_CODING:										// this tells us "9600,n,8,1" stuff (so we ignore)
						debug_print_string("USB_CDC_SET_LINE_CODING\r\n");

						usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, 0, 0);				// (ACK)
						usb_queue_td_out(USB_CDC_ENDPOINT_CONTROL);

						/*
							Print the packet contents
						*/
						{
						int ch;
						uint8_t *into = (uint8_t *)&our_cdc_line_coding;
						for (ch = 0; ch < setup_packet.wLength; ch++)
							{
							debug_print_hex((global_transfer_buffer[IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT][0])[ch]);
							*into++ = (global_transfer_buffer[IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT][0])[ch];
							debug_putc(' ');
							}
						debug_print_string("\r\n");
						}
						break;
					default:
						print_setup_packet(&setup_packet);
						debug_print_this("Unhandled device-to-host data direction setup request ", setup_packet.bRequest, " was received.");
					}
				}
			else
				{
				switch (setup_packet.bRequest)
					{
					case USB_REQUEST_SET_CONFIGURATION:
						debug_print_string("USB_REQUEST_SET_CONFIGURATION\r\n");
						usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, 0, 0);		// respond with a 0-byte transfer
						break;

					case USB_REQUEST_SET_ADDRESS:
						debug_print_string("USB_REQUEST_SET_ADDRESS\r\n");
						new_address = setup_packet.wValue;

						HW_USBCTRL_DEVICEADDR_WR(BF_USBCTRL_DEVICEADDR_USBADR(new_address) | BM_USBCTRL_DEVICEADDR_USBADRA);

						usb_queue_td_in(USB_CDC_ENDPOINT_CONTROL, 0, 0);		// respond with a 0-byte transfer

						usb_state = USB_DEVICE_STATE_ADDRESS;
						handled = 1;
						debug_print_this("                       Host has set our address to ", new_address, ".");
						usb_setup_endpoint_nonzero();
						break;
					default:
						//Packets with a host-to-device data phase direction
						print_setup_packet(&setup_packet);
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
				debug_print_this("  Queue Status:", global_dTD[endpt * 2].token.bit.status, "");
				debug_print_this("  Listen on ", endpt * 2, "");
				usb_queue_td_out(endpt);
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
				debug_print_hex((global_transfer_buffer[endpt * 2][0])[byte]);
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
				debug_print_hex((global_transfer_buffer[endpt * 2][0])[byte]);
				debug_print_string(" ");
				}
			debug_print_string("\r\n");

			managed = 1;

			if (endpt != 0)
				{
				debug_print_this("  Listen on Queuehead: ", endpt * 2, "");
				usb_queue_td_out(endpt);
				usb_queue_td_in(endpt, (char *)&((global_transfer_buffer[endpt * 2][0])[0]), 1);
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
			debug_print_hex(global_dTD[ep].token.bit.status);
			debug_print_string(")\r\n");
			}


		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_UEI);
		}
	else if (usb_status.B.UI)
		{
		debug_print_string(" [I: USB UI Interrupt ] \r\n");
		HW_USBCTRL_USBSTS_SET(BM_USBCTRL_USBSTS_UI);

		usb_interrupt();
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
imx_endpoint_transfer_descriptor *td;

td = &global_dTD[which];

memset(td, 0, sizeof(*td));
td->next_link_pointer = IMX_ENDPOINT_TRANSFER_DESCRIPTOR_TERMINATOR;

global_queuehead[which].dtd_overlay_area.next_link_pointer = (imx_endpoint_transfer_descriptor *)(((uint32_t)td) + 1);			// end of chain
global_queuehead[which].dtd_overlay_area.token.all = 0;
global_queuehead[which].dtd_overlay_area.token.bit.ioc = 1;
global_transfer_buffer[which][0] = (uint8_t *)main_memory_alloc(IMX_QUEUE_BUFFER_SIZE, IMX_QUEUE_BUFFER_SIZE);
global_queuehead[which].dtd_overlay_area.buffer_pointer[0] = (void *)global_transfer_buffer[which][0];
global_queuehead[which].capabilities.all = 0;
global_queuehead[which].capabilities.bit.maximum_packet_length = USB_MAX_PACKET_SIZE;
global_queuehead[which].capabilities.bit.ios = 1;
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
memset(global_queuehead, 0, (sizeof *global_queuehead));

/*
	Queue 0 and 1 : USB Control
*/
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_IN);				// EP 0

/*
	Hand off to the i.MX233
*/
HW_USBCTRL_ENDPOINTLISTADDR_SET((uint32_t)global_queuehead);
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
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_CONTROL_IN);				// EP 0

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

config_endpoint(IMX_USB_CDC_QUEUEHEAD_ACM_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_ACM_IN);
HW_USBCTRL_ENDPTCTRLn_WR(USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT, endpointcfg.U);		// EP 1

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

config_endpoint(IMX_USB_CDC_QUEUEHEAD_DATA_OUT);
config_endpoint(IMX_USB_CDC_QUEUEHEAD_DATA_IN);
HW_USBCTRL_ENDPTCTRLn_WR(USB_CDC_ENDPOINT_SERIAL, endpointcfg.U);		// EP 2


endpointcfg.U = 0;
HW_USBCTRL_ENDPTCTRLn_WR(3, endpointcfg.U);		// disable EP 3
endpointcfg.U = 0;
HW_USBCTRL_ENDPTCTRLn_WR(4, endpointcfg.U);		// disable EP 4

/*
	Hand off to the i.MX233
*/
HW_USBCTRL_ENDPOINTLISTADDR_SET((uint32_t)global_queuehead);

/*
	Prime the endpoints
*/
usb_queue_td_out(USB_CDC_ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT);
usb_queue_td_out(USB_CDC_ENDPOINT_SERIAL);
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

