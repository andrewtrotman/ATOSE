/*
	USB_CDC.H
	---------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_H_
#define USB_CDC_H_

#include <stdint.h>

/*
	class ATOSE_USB_CDC
	-------------------
	The purpose of this class is to give a namespace to the constants it contains
*/
class ATOSE_usb_cdc
{
public:
	static const uint16_t BCD_VERSION = 0x0110;			// We're compatible with version 1.1 of the CDC spec */

	/*
		USB CDC parameters
	*/
	static const uint8_t ABSTRACT_CONTROL = 0x02;
	static const uint8_t PROTOCOL_HAYES = 0x01;
	static const uint8_t PROTOCOL_NONE = 0x00;

	/*
		Messages from the host to the device
	*/
	static const uint8_t REQUEST_SEND_ENCAPSULATED_COMMAND = 0x00;
	static const uint8_t REQUEST_GET_ENCAPSULATED_RESPONSE = 0x01;
	static const uint8_t REQUEST_SET_COMM_FEATURE = 0x02;
	static const uint8_t REQUEST_GET_COMM_FEATURE = 0x03;
	static const uint8_t REQUEST_CLEAR_COMM_FEATURE = 0x04;
	static const uint8_t REQUEST_SET_LINE_CODING = 0x20;
	static const uint8_t REQUEST_GET_LINE_CODING = 0x21;
	static const uint8_t REQUEST_SET_CONTROL_LINE_STATE = 0x22;
	static const uint8_t REQUEST_SEND_BREAK = 0x23;

	/*
		Messages from the device to the host
	*/
	static const uint8_t NOTIFICATION_NETWORK_CONNECTION = 0x00;
	static const uint8_t NOTIFICATION_RESPONSE_AVAILABLE = 0x01;
	static const uint8_t NOTIFICATION_SERIAL_STATE = 0x20;


	/*
		The different descriptor subtypes we know about
	*/
	static const uint8_t DESCRIPTOR_SUBTYPE_HEADER = 0x00;
	static const uint8_t DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT = 0x01;
	static const uint8_t DESCRIPTOR_SUBTYPE_ABSTRACT_CONTROL_MANAGEMENT = 0x02;
	static const uint8_t DESCRIPTOR_SUBTYPE_UNION_FUNCTION = 0x06;
};

#endif
