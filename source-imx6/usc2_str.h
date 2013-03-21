/*
	USC2_STR.H
	----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef USC2_STR_H_
#define USC2_STR_H_

#include <stdint.h>

/*
	UCS2_TO_UTF8_CHARACTER()
	------------------------
	Convert a UCS-2 character into UTF-8.  A valid 3-byte buffer must be supplied to write into.
	Return the number of bytes needed to store the character in UTF-8

	This mehtod's code is based on that from: ftp://kermit.columbia.edu/kermit/charsets/utf8.c
	The original was written by F. da Cruz, Columbia University, May 2000. It did not contain
	a copyright notice when I found it.
*/
static inline uint32_t UCS2_to_utf8_character(uint8_t *utf8, uint16_t ucs2)
{
static uint8_t firstByteMark[] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
static const uint32_t byteMask = 0xBF;
static const uint32_t byteMark = 0x80;
uint32_t utf8len = 0;
uint32_t i = 0;

if (ucs2 < 0x80)
	utf8len = 1;
else if (ucs2 < 0x800)
	utf8len = 2;
else
	utf8len = 3;

i = utf8len;

switch (utf8len)
	{	                /* code falls through cases! */
	case 3:
		utf8[--i] = (ucs2 | byteMark) & byteMask;
		ucs2 >>= 6;
	case 2:
		utf8[--i] = (ucs2 | byteMark) & byteMask;
		ucs2 >>= 6;
	case 1: utf8[--i] =  ucs2 | firstByteMark[utf8len];
	}

return utf8len;
}

/*
	UCS2_TO_UTF8_STRLEN()
	---------------------
	return the number of bytes it would take to store this UCS-2 string once converted into UTF-8
*/
static inline uint32_t UCS2_to_utf8_strlen(uint16_t *ucs2)
{
uint16_t *from;
int32_t utf8_length = 0;

for (from = ucs2; *from != 0; from++)
	if (*ucs2 < 0x80)
		utf8_length++;
	else if (*ucs2 < 0x800)
		utf8_length += 2;
	else
		utf8_length += 3;

return utf8_length;
}

/*
	UCS2_STRLEN()
	-------------
	return the length of the UCS-2 string
*/
static inline uint32_t UCS2_strlen(uint16_t *ucs2)
{
uint16_t *from;

from = ucs2;
while (*from != 0)
	from++;

return from - ucs2;
}

/*
	UCS2_TO_UTF8_STRCPY()
	---------------------
	Copy the UCS-2 string and translate to UTF-8 on the way.  If the UTF-8 string would be
	longer than utf8_length then copy no characters and fail by returning NULL.
*/
static inline uint8_t *UCS2_to_utf8_strcpy(uint8_t *utf8, uint16_t *ucs2, uint32_t utf8_length)
{
uint16_t *from;
uint8_t *start = utf8;

/*
	Will it fit?
*/
if (UCS2_to_utf8_strlen(ucs2) > utf8_length)
	return NULL;

/*
	Copy it
*/
for (from = ucs2; *from != 0; from++)
	utf8 += UCS2_to_utf8_character(utf8, *from);

/*
	'\0' terminate the string
*/
*utf8 = '\0';

/*
	Done
*/
return start;
}

#endif
