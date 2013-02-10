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

/*
	ASCII_STRLEN()
	--------------
*/
static inline int ASCII_strlen(const char *string)
{
const char *ch;

ch = string;
while (*ch != '\0')
	ch++;

return ch - string;
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

#ifdef NEVER
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
#endif


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


#endif
