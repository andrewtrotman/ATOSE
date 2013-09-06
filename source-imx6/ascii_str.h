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
return __builtin_strlen(string);
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
return __builtin_strchr(string, value);
}

/*
	ASCII_STRRCHR()
	---------------
*/
static inline char *ASCII_strrchr(const char *string, int value)
{
return __builtin_strrchr(string, value);
}

/*
	ASCII_STRCPY()
	--------------
*/
static inline char *ASCII_strcpy(char *destination, const char *source)
{
return __builtin_strcpy(destination, source);
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
return __builtin_strncpy(destination, source, count);
}

/*
	ASCII_STRNCASECMP()
	-------------------
	this came from here:http://stackoverflow.com/questions/7299119/source-code-for-strncasecmp-function
	Where it is uncredited and so I assume it is unencumbered.
*/
static inline int ASCII_strncasecmp(const char *string_1, const char *string_2, size_t size)
{
return __builtin_strncasecmp(string_1, string_2, size);
}

/*
	ASCII_STRCMP()
	--------------
*/
static inline int ASCII_strcmp(const char *string_1, const char *string_2)
{
return __builtin_strcmp(string_1, string_2);
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
	ASCII_ATOI()
	------------
	from the ATIRE source code base
*/
static inline long ASCII_atol(char *string)
{
char *ch;
long ans = 0, multiplier;

if (*string == '-')
	{
	multiplier = -1;
	string++;
	}
else if (*string == '+')
	{
	multiplier = 1;
	string++;
	}
else
	multiplier = 1;

for (ch = string; *ch >= '0' && *ch <= '9'; ch++)
	ans = ans * 10 + (*ch - '0');

return multiplier * ans;
}

/*
	ASCII_ATOUL()
	-------------
	from the ATIRE source code base
*/
static inline unsigned long ASCII_atoul(char *string, size_t length)
{
char *ch;
unsigned long ans = 0;

for (ch = string; ((size_t)(ch - string) < length) && (*ch >= '0' && *ch <= '9'); ch++)
	ans = ans * 10 + (*ch - '0');

return ans;
}

/*
	ASCII_ATOLL()
	-------------
	from the ATIRE source code base
*/
static inline long long ASCII_atoll(char *string)
{
char *ch;
long long ans = 0, multiplier;

if (*string == '-')
	{
	multiplier = -1;
	string++;
	}
else if (*string == '+')
	{
	multiplier = 1;
	string++;
	}
else
	multiplier = 1;

for (ch = string; *ch >= '0' && *ch <= '9'; ch++)
	ans = ans * 10 + (*ch - '0');

return multiplier * ans;
}

/*
	MEMCPY()
	--------
*/
inline void *memcpy(void *destination, const void *source, size_t bytes)
{
#ifdef NEVER
	/*
		The built in memcpy does not understand memory allignmnt correctly and leads to crashes.
	*/
	return __builtin_memcpy(destination, source, bytes);
#else
	/*
		As __builtin_memcpy() on ARM is broken we have our own.
	*/
	uint8_t *from, *to, *end;

	from = (uint8_t *)source;
	to = (uint8_t *)destination;
	end = (uint8_t *)from + bytes;
	while (from < end)
			  *to++ = *from++;

	return destination;
#endif
}

/*
	MEMSET()
	--------
*/
inline void *memset(void *destination, int value, size_t bytes)
{
return __builtin_memset(destination, value, bytes);
}

/*
	BZERO()
	-------
*/
static inline void bzero(void *destination, size_t bytes)
{
__builtin_bzero(destination, bytes);
}

/*
	NONALIGNED()
	------------
*/
inline uint16_t nonaligned(const uint16_t *from)
{
uint8_t *byte;

byte = (uint8_t *)from;

return *byte | (*(byte + 1) << 8);
}

/*
	NONALIGNED()
	------------
*/
inline int16_t nonaligned(const int16_t *from)
{
uint8_t *byte;

byte = (uint8_t *)from;

return (int16_t)(*byte | (*(byte + 1) << 8));
}

#endif
