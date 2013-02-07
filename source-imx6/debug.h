/*
	DEBUG.H
	-------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Base class with various stream handeling methods for debugging purposes.
*/
#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdint.h>
#include "ascii_str.h"

/*
	class ATOSE_debug
	-----------------
*/
class ATOSE_debug
{
public:
	static const char eoln = '\n';

private:
	int base;

public:
	ATOSE_debug() {}

	virtual void init(void) { base = 10; }

	/*
		READ_BYTE()
		-----------
		read one byte from the I/O device and return the number of bytes read (0 = fail, 1 = success)
	*/
	virtual uint32_t read_byte(uint8_t *buffer) { return read(buffer, 1); }

	/*
		WRITE_BYTE()
		------------
		write one byte to the I/O device and return the number of bytes written (0 = fail, 1 = success)
	*/
	virtual uint32_t write_byte(const uint8_t buffer) { return write(&buffer, 1); }

	/*
		READ()
		------
		read a string of bytes from the input stream and return the number read (0 = fail)
	*/
	virtual uint32_t read(uint8_t *buffer, uint32_t bytes)
		{
		uint32_t gone;

		for (gone = 0; gone < bytes; gone++)
			if (read_byte(buffer++) == 0)
				return gone;

		return bytes;
		}

	/*
		WRITE()
		-------
		write a string of bytes to the output stream and return the number written (0 = fail)
	*/
	virtual uint32_t write(const uint8_t *buffer, uint32_t bytes)
		{
		uint32_t gone;

		for (gone = 0; gone < bytes; gone++)
			if (write_byte(*buffer++) == 0)
				return gone;

		return bytes;
		}

	/*
		PUTS()
		------
		print a string to the output stream and add a <cr> to the end
	*/
	virtual uint32_t puts(const uint8_t *message)
		{
		uint32_t length = ASCII_strlen((char *)message);

		write(message, length);
		write_byte('\n');

		return length + 1;
		}

	/*
		OPERATOR <<()
		-------------
		dump stuff to the output channel
	*/
	ATOSE_debug &operator << (const uint8_t character) 		{ write(&character, 1); return *this; }
	ATOSE_debug &operator << (const int8_t character) 			{ *this << (uint8_t)character; return *this; }
	ATOSE_debug &operator << (const char character) 			{ *this << (uint8_t)character; return *this; }

	ATOSE_debug &operator << (const uint8_t *string) 			{ write(string, ASCII_strlen((char *)string)); return *this; }
	ATOSE_debug &operator << (const int8_t *string) 			{ *this << (uint8_t *) string; return *this; }
	ATOSE_debug &operator << (const char *string) 				{ *this << (uint8_t *) string; return *this; }

	ATOSE_debug &operator << (uint64_t value) 					{ char buffer[sizeof(long) * 8 + 2]; ASCII_itoa(value, buffer, base); *this << buffer; return *this; }
	ATOSE_debug &operator << (int64_t value) 					{ char buffer[sizeof(long) * 8 + 2]; ASCII_itoa(value, buffer, base); *this << buffer; return *this; }

	ATOSE_debug &operator << (uint32_t value) 					{ *this << (uint64_t)value; return *this; }
	ATOSE_debug &operator << (int32_t value) 					{ *this << (int64_t)value; return *this; }

	ATOSE_debug &operator << (uint16_t value) 					{ *this << (uint64_t)value; return *this; }
	ATOSE_debug &operator << (int16_t value) 					{ *this << (int64_t)value; return *this; }

	/*
		HEX()
		-----
		Turn on hex output
	*/
	ATOSE_debug *hex(void) { base = 16; return this; }

	/*
		DECIMAL()
		---------
		Turn on decimal output
	*/
	ATOSE_debug *decimal(void) { base = 10; return this; }
} ;

#endif /* DEBUG_H_ */
