/*
	ASCII_STR.H
	-----------
	Copyright (c) 2012-2013 Andrew Trotman
	Licensed BSD

	String methods for ASCII strings (not Unicode strings)
*/
#ifndef ASCII_STR_H_
#define ASCII_STR_H_

#include <stddef.h>
#include <stdint.h>
#include "ctypes.h"

/*
	ASCII_STRLEN()
	--------------
*/
static inline uint32_t ASCII_strlen(const char *string)
{
const char *ch;

ch = string;
while (*ch != '\0')
	ch++;

return ch - string;
}

/*
	ASCII_STRLEN()
	--------------
*/
static inline uint32_t ASCII_strlen(const void *string) { return ASCII_strlen((const char *)string); }

/*
	ASCII_STRCHR()
	--------------
*/
static inline char *ASCII_strchr(const char *string, int value)
{
while (*string != '\0')
	{
	if (*string == value)
		return (char *)string;
	string++;
	}

return 0;
}

/*
	ASCII_STRRCHR()
	---------------
*/
static inline char *ASCII_strrchr(const char *string, int value)
{
char *here = 0;

while (*string != '\0')
	{
	if (*string == value)
		here = (char *)string;
	string++;
	}

return here;
}

/*
	ASCII_STRCPY()
	--------------
*/
static inline char *ASCII_strcpy(char *destination, const char *source)
{
char *into = (char *)destination;
char *from = (char *)source;

while ((*into++ = *from++) != '\0')
	;	// do nothing

return destination;
}

/*
	ASCII_STRCPY()
	--------------
*/
static inline char *ASCII_strcpy(void *destination, const void *source) { return ASCII_strcpy((char *)destination, (const char *)source); }

/*
	ASCII_STRNCPY()
	---------------
	This code comes from the free-BSD codebase
*/
static inline char *ASCII_strncpy(char *destination, const char *source, size_t count)
{
char *into = (char *)destination;
char *from = (char *)source;

if (count != 0)
	do
		if ((*into++ = *from++) == 0)
			{
			/* NUL pad the remaining n-1 bytes */
			while (--count != 0)
				*into++ = 0;
			break;
			}

	while (--count != 0);

return destination;
}

/*
	ASCII_STRNCASECMP()
	-------------------
	this came from here:http://stackoverflow.com/questions/7299119/source-code-for-strncasecmp-function
	Where it is uncredited and so I assume it is unencumbered.
*/
static inline int ASCII_strncasecmp(const char *string_1, const char *string_2, size_t size)
{
if (size == 0)
	return 0;

while (size-- != 0 && ASCII_tolower(*string_1) == ASCII_tolower(*string_2))
	{
	if (size == 0 || *string_1 == '\0' || *string_2 == '\0')
		break;
	string_1++;
	string_2++;
	}

return ASCII_tolower(*(unsigned char *)string_1) - ASCII_tolower(*(unsigned char *)string_2);
}

/*
	ASCII_STRCMP()
	--------------
*/
static inline int ASCII_strcmp(const char *string_1, const char *string_2)
{
while (*string_1 == *string_2 && *string_1 != '\0')
	{
	string_1++;
	string_2++;
	}

return (unsigned char)*string_1 - (unsigned char)*string_2;
}

/*
	ASCII_ITOA()
	------------
*/
static inline char *ASCII_itoa(int64_t value, char *destination, int base)
{
char sign, tmp;
char *into = destination;
char *from;

/*
	get the sign of the integer (is it -ve)
*/
if (value < 0)
	{
	sign = '-';
	value = -value;
	}
else
	sign = '\0';

/*
	do the digits in reverse order
*/
do
	{
	*into++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % base];
	value /= base;
	}
while (value > 0);

/*
	put the sign on
*/
if (sign != 0)
	*into++ = sign;

/*
	null terminate
*/
*into = '\0';

/*
	now reverse inplace
*/
into--;
for (from = destination; from < into; from++, into--)
	{
	tmp = *from;
	*from = *into;
	*into = tmp;
	}

return destination;
}

/*
	ASCII_ITOA()
	------------
*/
static inline char *ASCII_itoa(uint64_t value, char *destination, int base)
{
char tmp;
char *into = destination;
char *from;

/*
	do the digits in reverse order
*/
do
	{
	*into++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % base];
	value /= base;
	}
while (value > 0);

/*
	null terminate
*/
*into = '\0';

/*
	now reverse inplace
*/
into--;
for (from = destination; from < into; from++, into--)
	{
	tmp = *from;
	*from = *into;
	*into = tmp;
	}

return destination;
}

/*
	ASCII_ITOA()
	------------
*/
static inline char *ASCII_itoa(int32_t value, char *destination, int base) { return ASCII_itoa((int64_t)value, destination, base); }
static inline char *ASCII_itoa(uint32_t value, char *destination, int base) { return ASCII_itoa((uint64_t)value, destination, base); }

/*
	BZERO()
	-------
*/
static inline void bzero(void *destination, size_t bytes)
{
uint8_t *start, *end;

end = (uint8_t *)destination + bytes;

for (start = (uint8_t *)destination; start < end; start++)
	*start = 0;
}


/*
	MEMCPY()
	--------
*/
inline void *memcpy(void *destination, const void *source, size_t bytes)
{
uint8_t *from, *to, *end;

from = (uint8_t *)source;
to = (uint8_t *)destination;
end = (uint8_t *)from + bytes;
while (from < end)
	*to++ = *from++;

return destination;
}

/*
	MEMSET()
	--------
*/
inline void *memset(void *destination, int value, size_t bytes)
{
uint8_t *to, *end;

to = (uint8_t *)destination;
end = (uint8_t *)to + bytes;
while (to < end)
	*to++ = (char)value;

return destination;
}

/*
	NONALIGNED()
	------------
*/
inline uint16_t nonaligned(const uint16_t &from)
{
uint8_t *byte;

byte = (uint8_t *)&from;

return *byte  + (*(byte + 1) << 8);
}


#endif
