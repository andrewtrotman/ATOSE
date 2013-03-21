/*
	UTF8_STR.H
	----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef UTF8_STR_H_
#define UTF8_STR_H_

/*
	UTF8_BYTES()
	------------
	How many bytes does the UTF8 character take?
	The spec allows 5/6 byte sequences, but they are deemed invalid,
	so along with rubbish input we return the most that possibly
	could be taken, 8

	Our conversion functions will take care of the invalid cases
	This code comes from the ATIRE codebase (see atire.org)
*/
static inline uint32_t UTF8_bytes(const uint8_t *here)
{
if (*here < 0x80)				// 1-byte (ASCII) character
	return 1;
else if ((*here & 0xE0) == 0xC0)		// 2-byte sequence
	return 2;
else if ((*here & 0xF0) == 0xE0)		// 3-byte sequence
	return 3;
else if ((*here & 0xF8) == 0xF0)		// 4-byte sequence
	return 4;
return 8;
}

/*
	UTF8_ISCHAR()
	-------------
	Return true (the length of the character in bytes) if the given character is a valid UFT8 character, or 0 on fail
	This code comes from the ATIRE codebase (see atire.org)
*/
static inline uint32_t UTF8_ischar(const uint8_t *here)
{
uint32_t number_of_bytes = UTF8_bytes(here);
uint32_t byte;

/*
	While the UTF-8 spec allows for 5 or 6 byte sequences, they are invalid
	we might also get a sequence we don't know that we're interpreting as 8 ... which is also invalid
*/
if (number_of_bytes > 4)
	return 0;

for (byte = 1; byte < number_of_bytes; ++byte)
	if (((*(++here)) >> 6) != 2)
		return 0;

return number_of_bytes;
}

/*
	UTF8_TO_UTF32_CHARACTER()
	-------------------------
	Convert a UTF8 sequence into a UTF32.

	If the source buffer ends before the UTF-8 character is completed, zero is returned instead.
	This code comes from the ATIRE codebase (see atire.org)
*/
static inline uint32_t UTF8_to_utf32_character(const uint8_t *here, uint32_t *numchars)
{
switch (*numchars = UTF8_ischar(here))
	{
	case 0:
		return 0;
	case 1:
		return *here;
	case 2:
		//here[0] is known to be non-zero (since utf8_bytes(here) returned >1), so it can't be the terminator
		return ((*here & 0x1F) << 6) | (*(here + 1) & 0x3F);
	case 3:
		if (here[1])
			return ((*here & 0x0F) << 12) | ((*(here + 1) & 0x3F) << 6) | (*(here + 2) & 0x3F);
		else
			return 0;
	case 4:
		if (here[1] && here[2])
			return ((*here & 0x07) << 18) | ((*(here + 1) & 0x3F) << 12) | ((*(here + 2) & 0x3F) << 6) | (*(here + 3) & 0x3F);
		else
			return 0;
	default:
		return 0;
	}
}

/*
	UCS2_TO_UTF8_STRCPY()
	---------------------
	Copy the UTF-8 string and translate to UTF-32 on the way.
*/
static inline uint16_t *UTF8_to_ucs2_strcpy(uint16_t *ucs32, uint8_t *utf8)
{
uint8_t *from = utf8;
uint16_t *into = ucs32;
uint32_t character_length;
/*
	Copy it
*/
while ((*into++ = (uint16_t)UTF8_to_utf32_character(from, &character_length)) != 0)
	from += character_length;

return ucs32;
}


#endif
