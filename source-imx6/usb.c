/*
	USB.C
	-----
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#include <stdint.h>

#include "atose.h"

#include "usb.h"
#include "usb_setup_data.h"
#include "usb_string_descriptor.h"
#include "usb_string_descriptor_language.h"
#include "usb_standard_device_descriptor.h"
#include "usb_device_qualifier_descriptor.h"
#include "usb_cdc.h"
#include "usb_cdc_line_coding.h"
#include "usb_cdc_virtual_com_port.h"
#include "usb_ms.h"
#include "usb_ms_extended_properties.h"
#include "usb_ms_os_string_descriptor.h"
#include "usb_ms_extended_compatible_id_os_feature_descriptor.h"

/*
	Product and manufacturer IDs (allocated by the USB organisation)
*/
#define USB_ID_VENDOR							0xDEAD		// our vendor ID
#define USB_ID_PRODUCT 						0x000E		// device number
#define USB_ID_DEVICE							0x0100		// revision number

uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

/*
	OUR_ZERO
	--------
	for transmitting to the host
*/
static uint8_t our_zero_8 = 0;
static uint16_t our_zero_16 = 0;

/*
	OUR_ONE
	-------
	for transmitting to the host
*/
static uint8_t our_one_8 = 1;
static uint16_t our_one_16 = 1;

/*
	OUR_DEVICE_DESCRIPTOR
	---------------------
	We're going top fake being a serial port
*/
ATOSE_usb_standard_device_descriptor our_device_descriptor =
{
/*.bLength = */ sizeof(our_device_descriptor),
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_DEVICE,
/*.bcdUSB = */ ATOSE_usb::BCD_VERSION,						// USB 2.0
/*.bDeviceClass = */ ATOSE_usb::DEVICE_CLASS_CDC, 			// Communications Device Class (Pretend to be a Serial Port)
/*.bDeviceSubClass = */ ATOSE_usb_standard_device_descriptor::SUBCLASS_NONE,
/*.bDeviceProtocol = */ ATOSE_usb_standard_device_descriptor::PROTOCOL_NONE,
/*.bMaxPacketSize0 = */ ATOSE_usb::MAX_PACKET_SIZE, 		// Accept 64 bytes packet size on control endpoint (the maximum)
/*.idVendor = */ USB_ID_VENDOR,						// Manufacturer's ID
/*.idProduct = */ USB_ID_PRODUCT,					// Product's ID
/*.bcdDevice = */ USB_ID_DEVICE,					// Product Verison Number (revision number)
/*.iManufacturer = */ ATOSE_usb_string_descriptor::MANUFACTURER,		// string ID of the manufacturer
/*.iProduct = */ ATOSE_usb_string_descriptor::PRODUCT,				// string ID of the Product
/*.iSerialNumber = */ ATOSE_usb_string_descriptor::SERIAL_NUMBER,		// string ID of the serial number
/*.bNumConfigurations = */ 0x01
};

/*
	OUR_DEVICE_QUALIFIER
	--------------------
*/
ATOSE_usb_device_qualifier_descriptor our_device_qualifier =
{
/*.bLength = */ sizeof(our_device_descriptor),
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
/*.bcdUSB = */ ATOSE_usb::BCD_VERSION, 				// USB 2.0
/*.bDeviceClass = */ ATOSE_usb::DEVICE_CLASS_CDC, 			// Communications Device Class (Pretend to be a Serial Port)
/*.bDeviceSubClass = */ ATOSE_usb_standard_device_descriptor::SUBCLASS_NONE,
/*.bDeviceProtocol = */ ATOSE_usb_standard_device_descriptor::PROTOCOL_NONE,
/*.bMaxPacketSize0 = */ ATOSE_usb::MAX_PACKET_SIZE, 		// Accept 64 bytes packet size on control endpoint (the maximum)
/*.bNumConfigurations = */ 0x01,
/*.reserved = */ 0x00
} ;

/*
	OUR_LANGUAGE
	------------
*/
ATOSE_usb_string_descriptor_language our_language =
{
/*.bLength = */ sizeof(our_language),
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_STRING,
/*.wLANGID = */ ATOSE_usb_string_descriptor_language::ENGLISH_NEW_ZEALAND
};

/*
	OUR_SERIAL_NUMBER
	-----------------
*/
ATOSE_usb_string_descriptor our_serial_number =
{
/*.bLength = */ 14,
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_STRING,
/*.wString = */ {'S', 0x00, 'N', 0x00, '#', 0x00, 'D', 0x00, 'E', 0x00, 'V', 0x00}
};

/*
	OUR_MANUFACTURER
	----------------
*/
ATOSE_usb_string_descriptor our_manufacturer =
{
/*.bLength = */ 10,
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_STRING,
/*.wString = */ {'A', 0x00, 'S', 0x00, 'P', 0x00, 'T', 0x00}
} ;

/*
	OUR_PRODUCT
	-----------
*/
ATOSE_usb_string_descriptor our_product =
{
/*.bLength = */ 16,
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_STRING,
/*.wString = */ {'O', 0x00, 'c', 0x00, 't', 0x00, 'o', 0x00, 'p', 0x00, 'u', 0x00, 's', 0x00}
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
ATOSE_usb_cdc_virtual_com_port our_com_descriptor =
{
	{
	/*
		Configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_configuration_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION,
	/*.wTotalLength = */ sizeof(ATOSE_usb_cdc_virtual_com_port),
	/*.bNumInterfaces = */ 2,														// we have 2 interfaces: Abstract control management; and data
	/*.bConfigurationValue = */ 1,													// we are configuration 1
	/*.iConfiguration = */ ATOSE_usb_string_descriptor::NONE,
	/*.bmAttributes = */ ATOSE_usb_standard_configuration_descriptor::SELFPOWERED,	// selfpowered
	/*.bMaxPower = */ ATOSE_usb_standard_configuration_descriptor::mA_100			// we draw no more than 100mA
	},
	{
	/*
		Interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_interface_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_INTERFACE,
	/*.bInterfaceNumber = */ 0,												// control on interface 0
	/*.bAlternateSetting = */ 0,
	/*.bNumEndpoints = */ 1,													// uses 1 endpoint
	/*.bInterfaceClass = */ ATOSE_usb_standard_interface_descriptor::CLASS_CDC,
	/*.bInterfaceSubClass = */ ATOSE_usb_cdc::ABSTRACT_CONTROL,				// Abstract Control
	/*.bInterfaceProtocol = */ ATOSE_usb_cdc::PROTOCOL_HAYES,				// using Hayes AT command set
	/*.iInterface = */ 0
	},
	{
	/*
		CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
	*/
	/*.bLength = */ sizeof(ATOSE_usb_cdc_header_functional_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_CS_INTERFACE,
	/*.bDescriptorSubType = */ ATOSE_usb_cdc::DESCRIPTOR_SUBTYPE_HEADER,
	/*.bcdCDC = */ ATOSE_usb_cdc::BCD_VERSION
	},
	{
	/*
		Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
	*/
	/*.bLength = */ sizeof(ATOSE_usb_cdc_call_management_functional_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_CS_INTERFACE,
	/*.bDescriptorSubType = */ ATOSE_usb_cdc::DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT,
	/*.bmCapabilities = */ ATOSE_usb_cdc_call_management_functional_descriptor::CAPABILITY_CM_COMUNICATIONS,				// we do call management over the Communicatons Class Interface
	/*.bDataInterface = */ 1													// data class interface is interface 1
	},
	{
	/*
		Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
	*/
	/*.bLength = */ sizeof(ATOSE_usb_cdc_abstract_control_management_functional_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_CS_INTERFACE,
	/*.bDescriptorSubType = */ ATOSE_usb_cdc::DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT,
	/*.bmCapabilities = */ ATOSE_usb_cdc_abstract_control_management_functional_descriptor::CAPABILITY_BREAK | ATOSE_usb_cdc_abstract_control_management_functional_descriptor::CAPABILITY_LINE	// we must manage Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, Serial_State, and Send_Break
	},
	{
	/*
		Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
	*/
	/*.bLength = */ sizeof(ATOSE_usb_union_interface_functional_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_CS_INTERFACE,
	/*.bDescriptorSubType = */ ATOSE_usb_cdc::DESCRIPTOR_SUBTYPE_UNION_FUNCTION,
	/*.bMasterInterface = */ 0,												// interface 0 is for control
	/*.bSlaveInterface0 = */ 1												// interface 1 is for other stuff
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_endpoint_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_ENDPOINT,
	/*.bEndpointAddress = */ ATOSE_usb_cdc_virtual_com_port::ENDPOINT_ABSTRACT_CONTROL_MANAGEMENT | ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN,
	/*.bmAttributes = */ ATOSE_usb_standard_endpoint_descriptor::TYPE_INTERRUPT,						// notifications on an INTERRUPT ENDPOINT
	/*.wMaxPacketSize = */ ATOSE_usb::MAX_PACKET_SIZE,
	/*.bInterval = */ 10
	},
	{
	/*
		Interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_interface_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_INTERFACE,
	/*.bInterfaceNumber = */ 1,												// data on interface 1
	/*.bAlternateSetting = */ 0,
	/*.bNumEndpoints = */ 2,													// uses 2 endpoints
	/*.bInterfaceClass = */ ATOSE_usb_standard_interface_descriptor::CLASS_DATA,						// data
	/*.bInterfaceSubClass = */ ATOSE_usb_standard_device_descriptor::SUBCLASS_NONE,
	/*.bInterfaceProtocol = */ ATOSE_usb_cdc::PROTOCOL_NONE,
	/*.iInterface = */ 0
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_endpoint_descriptor),
	/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_ENDPOINT,
	/*.bEndpointAddress = */ ATOSE_usb_cdc_virtual_com_port::ENDPOINT_SERIAL | ATOSE_usb_standard_endpoint_descriptor::DIRECTION_OUT,
	/*.bmAttributes = */ ATOSE_usb_standard_endpoint_descriptor::TYPE_BULK,
	/*.wMaxPacketSize = */ ATOSE_usb::MAX_PACKET_SIZE,
	/*.bInterval = */ 0														// this endpoint does not NAK
	},
	{
	/*
		Endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	*/
	/*.bLength = */ sizeof(ATOSE_usb_standard_endpoint_descriptor),
	/*.bDescriptorType = */ 	ATOSE_usb::DESCRIPTOR_TYPE_ENDPOINT,
	/*.bEndpointAddress = */ ATOSE_usb_cdc_virtual_com_port::ENDPOINT_SERIAL | ATOSE_usb_standard_endpoint_descriptor::DIRECTION_IN,
	/*.bmAttributes = */ ATOSE_usb_standard_endpoint_descriptor::TYPE_BULK,
	/*.wMaxPacketSize = */ ATOSE_usb::MAX_PACKET_SIZE,
	/*.bInterval = */ 0														// this endpoint does not NAK
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
ATOSE_usb_cdc_line_coding our_cdc_line_coding =
{
/*.dwDTERat = */ 115200,
/*.bCharFormat = */ ATOSE_usb_cdc_line_coding::STOP_BITS_1,
/*.bParityType = */  ATOSE_usb_cdc_line_coding::PARITY_NONE,
/*.bDataBits = */  8
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
ATOSE_usb_ms_os_string_descriptor our_ms_os_string_descriptor =
{
/*.bLength = */ sizeof(our_ms_os_string_descriptor),
/*.bDescriptorType = */ ATOSE_usb::DESCRIPTOR_TYPE_STRING,
/*.qwSignature = */ {'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, '1', 0x00, '0', 0x00, '0', 0x00},		// MSFT100
/*.bMS_VendorCode = */ 0,																				// this is a user-defined value and appears to be pretty much useless
/*.bPad = */ 0
};


/*
	OUR_MS_COMPATIBLE_ID_FEATURE_DESCRIPTOR
	---------------------------------------
*/
ATOSE_usb_ms_extended_compatible_id_os_feature_descriptor our_ms_compatible_id_feature_descriptor =
{
	{
	/*.dwLength = */ sizeof(our_ms_compatible_id_feature_descriptor),
	/*.bcdVersion = */ ATOSE_usb_ms::BCD_VERSION,
	/*.wIndex = */ ATOSE_usb_ms::DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID,
	/*.bCount = */ 1,
	/*.reserved = */ {0},
	},
	{
	/*.bFirstInterfaceNumber = */ 0,
	/*.reserved1 = */ 0,
	/*.compatibleID = */ {0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00},	// WINUSB
	/*.subCompatibleID = */ {0},
	/*.reserved2 = */ {0}
	}
} ;


/*
	OUR_MS_EXTENDED_PROPERTIES
	--------------------------
*/
ATOSE_usb_ms_extended_properties our_ms_extended_properties =
{
	{
	/*.dwLength = */ sizeof(our_ms_extended_properties),
	/*.bcdVersion = */ ATOSE_usb_ms::BCD_VERSION,
	/*.wIndex = */ ATOSE_usb_ms::DESCRIPTOR_TYPE_EXTENDED_PROPERTIES,
	/*.wCount = */ 1															// 1 custom property
	},
	{
	/*.dwSize = */ sizeof(ATOSE_usb_ms_extended_property_function),
	/*.dwPropertyDataType = */ ATOSE_usb_ms::REG_SZ,
	/*.wPropertyNameLength = */ 12,
	/*.bPropertyName = */ {'L', 0x00, 'a', 0x00, 'b', 0x00, 'e', 0x00, 'l', 0x00, 0x00, 0x00},
	/*.dwPropertyDataLength = */ 16,
	/*.bPropertyData = */ {'O', 0x00, 'c', 0x00, 't', 0x00, 'o', 0x00, 'p', 0x00, 'u', 0x00, 's', 0x00, 0x00, 0x00}
	}
} ;

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
//int usb_state = USB_DEVICE_STATE_DEFAULT;		// unused

/*
	ATOSE_USB::ATOSE_USB()
	----------------------
*/
ATOSE_usb::ATOSE_usb() : ATOSE_device_driver()
{
}

/*
	ATOSE_USB::ACK()
	----------------
	Send a software ACK to acknowledge the processing of a command

	The hardware ACK is taken care of by the i.MX233.
	See table 64-69 page 5349 of "i.MX 6Dual/6Quad Applications Processor Reference Manual Rev. 0, 11/2012"

	The sofware response ACK is a zero-length packet.
	See page 227 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
*/
void ATOSE_usb::ack(uint32_t endpoint)
{
send_to_host(endpoint, 0, 0);		// send a 0-length packet
}

/*
	====================
	Generic USB routines
	====================
*/

/*
	ATOSE_USB::DEBUG_PRINT_STRING()
	-------------------------------
*/
void ATOSE_usb::debug_print_string(const char *string)
{
ATOSE_atose::get_ATOSE()->debug << string;
}

/*
	ATOSE_USB::DEBUG_PRINT_THIS()
	-----------------------------
*/
void ATOSE_usb::debug_print_this(const char *string, uint32_t number, const char *string2)
{
ATOSE_atose::get_ATOSE()->debug << string << number << string2 << "\r\n";
}

/*
	ATOSE_USB::PRINT_SETUP_PACKET()
	-------------------------------
*/
void ATOSE_usb::print_setup_packet(ATOSE_usb_setup_data *packet)
{
debug_print_string("SETUP PACKET\r\n");
debug_print_this("bmRequestType:", packet->bmRequestType.all, "");
debug_print_this("bRequest:", packet->bRequest, "");
debug_print_this("wValue:", packet->wValue, "");
debug_print_this("wIndex:", packet->wIndex, "");
debug_print_this("wLength:", packet->wLength, "");
}

/*
	ATOSE_USB::USB_GET_DESCRIPTOR()
	-------------------------------
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
void ATOSE_usb::usb_get_descriptor(ATOSE_usb_setup_data *packet)
{
uint32_t descriptor_index;
uint32_t descriptor_type = packet->wValue >> 8;

switch (descriptor_type)
	{
	case ATOSE_usb::DESCRIPTOR_TYPE_DEVICE:
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
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_device_descriptor, packet->wLength != sizeof(our_device_descriptor) ? 8 : sizeof(our_device_descriptor));
		break;
	case ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_CONFIGURATION\r\n");
		/*
			This descriptor is shared with USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION, so we set its type before sending

			For more information on the configuration descriptor see page 264-266 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		our_com_descriptor.cd.bDescriptorType = ATOSE_usb::DESCRIPTOR_TYPE_CONFIGURATION;
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_com_descriptor, min(packet->wLength, sizeof(our_com_descriptor)));
		break;

	case ATOSE_usb::DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
		debug_print_string("USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION\r\n");
		/*
			The OTHER_SPEED_CONFIGURATION request is the same as a configurataion request but allows us to have a device that
			works differently at different speeds.  We ignore this and make the device work the same way regardless of speed.
			To do this we just change the descriptor type.

			For more information on the other speed configuration descriptor see page 266-267 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		our_com_descriptor.cd.bDescriptorType = ATOSE_usb::DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION;
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_com_descriptor, min(packet->wLength, sizeof(our_com_descriptor)));
		break;

	case ATOSE_usb::DESCRIPTOR_TYPE_STRING:
		descriptor_index = packet->wValue & 0xFF;

		/*
			For more information on string descriptors see page 273-274 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		switch (descriptor_index)
			{
			case ATOSE_usb_string_descriptor::LANGUAGE:
				debug_print_string("STRING - LANG ID\r\n");
				/*
					Here the host is actaully asking for the languages we support rather than a string
					"String index zero for all languages returns a string descriptor that contains an array
					of two-byte LANGID codes supported by the device"

					See page 273 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
				*/
				send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_language, min(packet->wLength, our_language.bLength));
				break;
			case ATOSE_usb_string_descriptor::MANUFACTURER:
				debug_print_string("STRING - MANUFACTURER\r\n");
				send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_manufacturer, min(packet->wLength, our_manufacturer.bLength));
				break;
			case ATOSE_usb_string_descriptor::PRODUCT:
				debug_print_string("STRING - PRODUCT\r\n");
				send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_product, min(packet->wLength, our_product.bLength));
				break;
			case ATOSE_usb_string_descriptor::SERIAL_NUMBER:
				debug_print_string("STRING - SERIAL NUMBER\r\n");
				send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_serial_number, min(packet->wLength, our_serial_number.bLength));
				break;
			case ATOSE_usb_ms::STRING_OS_DESCRIPTOR:
				debug_print_string("MS OS STRING DESCRIPTOR\r\n");
				/*
					Microsoft OS String Desciptor
				*/
				send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_ms_os_string_descriptor, sizeof(our_ms_os_string_descriptor));
				break;
			default:
				/*
					If the host is behaving correting this cannot happen
				*/
				debug_print_this("Unhandled STRING DESCRIPTOR:" , descriptor_index, "");
				signal_an_error(0);
				break;
			}
		break;
	case ATOSE_usb::DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
		debug_print_string("USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER\r\n");
		/*
			"The device_qualifier descriptor describes information about a high-speed capable device that would
			change if the device were operating at the other speed".
			See page 264 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
		*/
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_device_qualifier, min(packet->wLength, sizeof(our_device_qualifier)));
		break;
	default:
		/*
			If the host is behaving correting this cannot happen
		*/
		debug_print_string("Unrecognised setup packet (illegal descriptor):\r\n");
		print_setup_packet(packet);
		signal_an_error(0);
		break;
	}
}

/*
	ATOSE_USB::MS_USB_GET_DESCRIPTOR()
	----------------------------------
	The behaviour of this method is defined in two documents: "Extended Properties OS Feature Descriptor Specification,
	July 13, 2012" and "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012".

	Microsoft supports 3 descriptor types (passed in wIndex), but type 0x01 (Genre) is described in the July 13, 2012 spec as
	"This descriptor is being considered for future versions of Windows, and no specification is currently available" (page 4
	of "Microsoft OS Descriptors Overview, July 13, 2012")

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t ATOSE_usb::ms_usb_get_descriptor(ATOSE_usb_setup_data *packet)
{
switch (packet->wIndex)
	{
	case ATOSE_usb_ms::DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID:
		debug_print_string("MS_USB_DESCRIPTOR_TYPE_EXTENDED_COMPAT_ID\r\n");
		/*
			See: "Extended Compat ID OS Feature Descriptor Specification, July 13, 2012".
		*/
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_ms_compatible_id_feature_descriptor, sizeof(our_ms_compatible_id_feature_descriptor));
		return 1;
	case ATOSE_usb_ms::DESCRIPTOR_TYPE_EXTENDED_PROPERTIES:
		debug_print_string("MS_USB_DESCRIPTOR_TYPE_EXTENDED_PROPERTIES\r\n");
		/*
			See: Extended Properties OS Feature Descriptor Specification, July 13, 2012"
		*/
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_ms_extended_properties, sizeof(our_ms_extended_properties));
		return 1;
	default:
		debug_print_string("Unrecognised Microsoft descriptor request:\r\n");
		print_setup_packet(packet);
		signal_an_error(0);
		return 1;
	}

return 0;
}

/*
	ATOSE_USB::USB_COMMAND()
	------------------------
	The methods we must implement are descibed on page 250 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
	All of them are listed here, even if not implemented (because we can ignore some).

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t ATOSE_usb::usb_command(ATOSE_usb_setup_data *packet)
{
uint8_t *what_to_send;

switch (packet->bRequest)
	{
	case ATOSE_usb::REQUEST_GET_STATUS:
		debug_print_string("USB_REQUEST_GET_STATUS\r\n");
		/*
			See page 254 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = (packet->bmRequestType.bit.recipient == ATOSE_usb_setup_data::RECIPIENT_DEVICE) ? (uint8_t *)&our_one_16 : (uint8_t *)&our_zero_16;
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, what_to_send, sizeof(uint16_t));
		return 1;

	case ATOSE_usb::REQUEST_CLEAR_FEATURE:
		debug_print_string("USB_REQUEST_CLEAR_FEATURE\r\n");
		/*
			See page 252 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		ack(0);
		return 1;

	case ATOSE_usb::REQUEST_SET_FEATURE:
		debug_print_string("USB_REQUEST_SET_FEATURE\r\n");
		/*
			See page 258 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		ack(0);
		return 1;

	case ATOSE_usb::REQUEST_SET_ADDRESS:
		debug_print_this("USB_REQUEST_SET_ADDRESS(", packet->wValue, ")");
		/*
			See page 256 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		set_address(packet->wValue);
		ack(0);

		enable_endpoint(1, ATOSE_usb_standard_endpoint_descriptor::TYPE_INTERRUPT);
		enable_endpoint(2, ATOSE_usb_standard_endpoint_descriptor::TYPE_BULK);
		return 1;

	case ATOSE_usb::REQUEST_GET_DESCRIPTOR:
		debug_print_string("USB_REQUEST_GET_DESCRIPTOR: ");
		/*
			See page 253 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		usb_get_descriptor(packet);
		return 1;

	case ATOSE_usb::REQUEST_SET_DESCRIPTOR:
		debug_print_string("USB_REQUEST_SET_DESCRIPTOR\r\n");
		/*
			See page 257 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		ack(0);
		return 1;

	case ATOSE_usb::REQUEST_GET_CONFIGURATION:
		debug_print_string("USB_REQUEST_GET_CONFIGURATION\r\n");
		/*
			See page 253 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = &our_one_8;
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, what_to_send, sizeof(uint8_t));
		return 1;

	case ATOSE_usb::REQUEST_SET_CONFIGURATION:
		debug_print_string("USB_REQUEST_SET_CONFIGURATION\r\n");
		/*
			See page 257 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		ack(0);
		return 1;

	case ATOSE_usb::REQUEST_GET_INTERFACE:
		debug_print_string("USB_REQUEST_GET_INTERFACE\r\n");
		/*
			See page 254 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		what_to_send = &our_zero_8;
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, what_to_send, sizeof(uint8_t));
		return 1;

	case ATOSE_usb::REQUEST_SET_INTERFACE:
		debug_print_string("USB_REQUEST_SET_INTERFACE\r\n");
		/*
			See page 259 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		signal_an_error(0);
		return 1;

	case ATOSE_usb::REQUEST_SYNCH_FRAME:
		debug_print_string("USB_REQUEST_SYNCH_FRAME\r\n");
		/*
			See page 260 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000".
		*/
		signal_an_error(0);
		return 1;
	}

return 0;
}

/*
	ATOSE_UB::USB_CDC_COMMAND()
	---------------------------
	Manage the CDC commands (of which there aren't many).
	Here we manage only those requests necessary for a CDC virtual COM port

	returns 1 if the message was processes, 0 if it was not
*/
uint32_t ATOSE_usb::usb_cdc_command(ATOSE_usb_setup_data *packet)
{
switch (packet->bRequest)
	{
	case ATOSE_usb_cdc::REQUEST_SET_LINE_CODING:
		debug_print_string("USB_CDC_REQUEST_SET_LINE_CODING\r\n");
		/*
			See page 57 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		ack(0);
		return 1;

	case ATOSE_usb_cdc::REQUEST_GET_LINE_CODING:
		debug_print_string("USB_CDC_REQUEST_GET_LINE_CODING\r\n");
		/*
			See page 58 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		send_to_host(ATOSE_usb_cdc_virtual_com_port::ENDPOINT_CONTROL, (uint8_t *)&our_cdc_line_coding, sizeof(our_cdc_line_coding));
		return 1;

	case ATOSE_usb_cdc::REQUEST_SET_CONTROL_LINE_STATE:
		debug_print_string("USB_CDC_REQUEST_SET_CONTROL_LINE_STATE\r\n");
		/*
			See page 58 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		ack(0);
		return 1;

	case ATOSE_usb_cdc::REQUEST_SEND_BREAK:
		debug_print_string("USB_CDC_REQUEST_SEND_BREAK\r\n");
		/*
			See page 59 of "Universal Serial Bus Class Definitions for Communication Devices Version 1.1"
		*/
		ack(0);
		return 1;
	}

return 0;
}

/*
	ATOSE_USB::PROCESS_SETUP_PACKET()
	---------------------------------
*/
void ATOSE_usb::process_setup_packet(ATOSE_usb_setup_data *setup_packet)
{
/*
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
if (setup_packet->bmRequestType.bit.type == ATOSE_usb_setup_data::TYPE_STANDARD)
	usb_command(setup_packet);
else if (setup_packet->bmRequestType.bit.type == ATOSE_usb_setup_data::TYPE_CLASS)
	usb_cdc_command(setup_packet);
else if (setup_packet->bmRequestType.bit.type == ATOSE_usb_setup_data::TYPE_VENDOR)
	ms_usb_get_descriptor(setup_packet);
}
