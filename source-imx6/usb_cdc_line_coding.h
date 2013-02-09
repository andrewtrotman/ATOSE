/*
	USB_CDC_LINE_CODING.H
	---------------------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USB_CDC_LINE_CODING_H_
#define USB_CDC_LINE_CODING_H_

#include <stdint.h>

/*
	class ATOSE_USB_CDC_LINE_CODING
	-------------------------------
*/
class ATOSE_usb_cdc_line_coding
{
public:
	static const uint8_t STOP_BITS_1 = 0x00;
	static const uint8_t STOP_BITS_15 = 0x01;
	static const uint8_t STOP_BITS_2 = 0x02;

	static const uint8_t PARITY_NONE = 0x00;
	static const uint8_t PARITY_ODD = 0x01;
	static const uint8_t PARITY_EVEN = 0x02;
	static const uint8_t PARITY_MARK = 0x03;
	static const uint8_t PARITY_SPACE = 0x04;

public:
	uint32_t dwDTERat;		// Number Data terminal rate, in bits per second.
	uint8_t bCharFormat;	// Number Stop bits (0 = 1 Stop bit, 1 = 1.5 Stop bits, 2 = 2 Stop bits)
	uint8_t bParityType;	// Number Parity (0 = None, 1 = Odd, 2 = Even, 3 = Mark, 4 = Space)
	uint8_t bDataBits;		// Number Data bits (5, 6, 7, 8 or 16).
} __attribute__ ((packed));

#endif

