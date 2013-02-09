/*
	DEBUG.C
	-------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	Not much is needed here other than routines to make it easier to subclass
	that is, if you define a write() method that outputs one character than you
	automaticaly get writing of strings, but if you override the output strings
	you automatically get outputting a single character.
*/
#include "debug.h"

/*
	End of line on the debug terminal is the crlf combination.  Unfortunately, Windows, Macintosh and
	Linux use different line end combinations; as I'm currently on Windows with stick to crlf.
*/
const uint8_t *ATOSE_debug::eoln = (uint8_t *)"\r\n";

/*
	ATOSE_DEBUG::READ()
	-------------------
	read a string of bytes from the input stream and return the number read (0 = fail)
*/
uint32_t ATOSE_debug::read(uint8_t *buffer, uint32_t bytes)
{
uint32_t gone;

for (gone = 0; gone < bytes; gone++)
	if (read_byte(buffer++) == 0)
		return gone;

return bytes;
}

/*
	ATOSE_DEBUG::WRITE()
	--------------------
	write a string of bytes to the output stream and return the number written (0 = fail)
*/
uint32_t ATOSE_debug::write(const uint8_t *buffer, uint32_t bytes)
{
uint32_t gone;

for (gone = 0; gone < bytes; gone++)
	if (write_byte(*buffer++) == 0)
		return gone;

return bytes;
}
