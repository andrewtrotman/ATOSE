/*
	IO.H
	----
*/
#ifndef IO_H_
#define IO_H_

#include "ascii_str.h"

/*
	class ATOSE_IO
	--------------
*/

class ATOSE_IO
{
public:
	static const char eoln = '\n';

private:
	int base;

public:
	ATOSE_IO() { base = 10; }
	virtual ~ATOSE_IO() {}

	/*
		READ_BYTE()
		-----------
		read one byte from the I/O device and return the number of bytes read (0 = fail, 1 = success)
	*/
	virtual int read_byte(char *buffer) { return read(buffer, 1); }

	/*
		WRITE_BYTE()
		------------
		write one byte to the I/O device and return the number of bytes written (0 = fail, 1 = success)
	*/
	virtual int write_byte(const char buffer) { return write(&buffer, 1); }

	/*
		READ()
		------
		read a string of bytes from the input stream and return the number read (0 = fail)
	*/
	virtual int read(char *buffer, int bytes)
		{
		int gone;

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
	virtual int write(const char *buffer, int bytes)
		{
		int gone;

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
	virtual int puts(const char *message)
		{
		write(message, ASCII_strlen(message));
		write_byte('\n');
		}

	/*
		OPERATOR <<()
		-------------
		dump stuff to the outout channel
	*/
	ATOSE_IO &operator << (const char character) { write(&character, 1); return *this; }
	ATOSE_IO &operator << (const unsigned char character) { *this << (char)character; return *this; }

	ATOSE_IO &operator << (const char *string) { write(string, ASCII_strlen(string)); return *this; }
	ATOSE_IO &operator << (const unsigned char *string) { *this << (const char *)string; return *this; }
	ATOSE_IO &operator << (long value) { char buffer[sizeof(long) * 8 + 2]; ASCII_itoa(value, buffer, base); *this << buffer; return *this; }

	/*
		HEX()
		-----
		Turn on hex output
	*/
	ATOSE_IO *hex(void) { base = 16; return this; }

	/*
		DECIMAL()
		---------
		Turn on decimal output
	*/
	ATOSE_IO *decimal(void) { base = 16; return this; }
} ;

#endif /* IO_H_ */

