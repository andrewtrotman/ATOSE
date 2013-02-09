/*
	USB.H
	-----
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_H_
#define USB_H_

#include <stdint.h>

/*
	class ATOSE_USB
	---------------
*/
class ATOSE_usb
{
public:
	/*
		We're compatible with: USB 2.0
		Recall that all directions are RELATIVE TO THE HOST.
	*/
	static const uint16_t BCD_VERSION 						0x0200		/* we're compliant with USB 2.0 */

	/*
		States that the USB device hardware can be in
		See page 243 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
	*/
	static const uint8_t DEVICE_STATE_ATTACHED = 0x00;
	static const uint8_t DEVICE_STATE_POWERED = 0x01;
	static const uint8_t DEVICE_STATE_DEFAULT = 0x02;
	static const uint8_t DEVICE_STATE_ADDRESS = 0x03;
	static const uint8_t DEVICE_STATE_CONFIGURED = 0x04;
	static const uint8_t DEVICE_STATE_SUSPENDED = 0x05;

	/*
		Configurable parameters
	*/
	static const uint8_t USB_MAX_PACKET_SIZE = 64;			// under USB 2.0 this must be 8, 16, 32, or 64


	/*
		The different descriptor types we know about
	*/
	static const uint8_t DESCRIPTOR_TYPE_DEVICE = 0x01;
	static const uint8_t DESCRIPTOR_TYPE_CONFIGURATION = 0x02;
	static const uint8_t DESCRIPTOR_TYPE_STRING = 0x03;
	static const uint8_t DESCRIPTOR_TYPE_INTERFACE = 0x04;
	static const uint8_t DESCRIPTOR_TYPE_ENDPOINT = 0x05;
	static const uint8_t DESCRIPTOR_TYPE_DEVICE_QUALIFIER = 0x06;
	static const uint8_t DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION = 0x07;
	static const uint8_t DESCRIPTOR_TYPE_INTERFACE_POWER = 0x08;
	static const uint8_t DESCRIPTOR_TYPE_CS_INTERFACE = 0x24;

	/*
		The different device classes we know about
	*/
	static const uint8_t DEVICE_CLASS_CDC = 0x02;

	/*
		Standard USB methods on endpoint 0
		See page 251 of "Universal Serial Bus Specification Revision 2.0 April 27, 2000"
	*/
	static const uint8_t REQUEST_GET_STATUS = 0x00;
	static const uint8_t REQUEST_CLEAR_FEATURE = 0x01;
	static const uint8_t REQUEST_RESERVED0x02 = 0x02;
	static const uint8_t REQUEST_SET_FEATURE = 0x03;
	static const uint8_t REQUEST_RESERVED0x04 = 0x04;
	static const uint8_t REQUEST_SET_ADDRESS = 0x05;
	static const uint8_t REQUEST_GET_DESCRIPTOR = 0x06;
	static const uint8_t REQUEST_SET_DESCRIPTOR = 0x07;
	static const uint8_t REQUEST_GET_CONFIGURATION = 0x08;
	static const uint8_t REQUEST_SET_CONFIGURATION = 0x09;
	static const uint8_t REQUEST_GET_INTERFACE = 0x0A;
	static const uint8_t REQUEST_SET_INTERFACE = 0x0B;
	static const uint8_t REQUEST_SYNCH_FRAME = 0x0C;

	/*
		USB features
	*/
	static const uint8_t FEATURE_ENDPOINT_HALT = 0x00;
	static const uint8_t FEATURE_DEVICE_REMOTE_WAKEUP = 0x01;
	static const uint8_t FEATURE_TEST_MODE = 0x02;
} ;


#endif /* USB_H_ */
