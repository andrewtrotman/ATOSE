/*
	CTYPES.H
	--------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef CTYPES_H_
#define CTYPES_H_

enum
	{
	ATOSE_CTYPE_UPPER = 1,
	ATOSE_CTYPE_LOWER = 2,
	ATOSE_CTYPE_DIGIT = 4,
	ATOSE_CTYPE_CONTROL = 8,
	ATOSE_CTYPE_PUNC = 16,
	ATOSE_CTYPE_SPACE = 32,
	ATOSE_CTYPE_HEX = 64,
	ATOSE_CTYPE_HARD_SPACE = 128, 		// character 0x20

	ATOSE_CTYPE_ISALPHA = ATOSE_CTYPE_UPPER | ATOSE_CTYPE_LOWER,
	ATOSE_CTYPE_ISALNUM = ATOSE_CTYPE_ISALPHA | ATOSE_CTYPE_DIGIT,
	ATOSE_CTYPE_ISGRAPH  = ATOSE_CTYPE_PUNC | ATOSE_CTYPE_UPPER | ATOSE_CTYPE_LOWER | ATOSE_CTYPE_DIGIT,
	ATOSE_CTYPE_ISPRINT = ATOSE_CTYPE_PUNC | ATOSE_CTYPE_ISALNUM | ATOSE_CTYPE_HARD_SPACE,
	} ;

extern uint8_t ATOSE_ctype[];
extern uint8_t ATOSE_toupper_list[];
extern uint8_t ATOSE_tolower_list[];

/*
	ATOSE_TO_CTYPE()
	--------------
	do the cast and cause a lookup to the lookup table
*/
inline uint32_t ATOSE_to_ctype(uint32_t x) { return ATOSE_ctype[(uint8_t)x]; }

/*
	IS() routines
*/
inline uint32_t ASCII_islower(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_LOWER) != 0; }
inline uint32_t ASCII_isupper(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_UPPER) != 0; }
inline uint32_t ASCII_isalpha(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_ISALPHA) != 0; }
inline uint32_t ASCII_isdigit(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_DIGIT) != 0; }
inline uint32_t ASCII_isxdigit(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_HEX) != 0; }
inline uint32_t ASCII_isalnum(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_ISALNUM) != 0; }
inline uint32_t ASCII_isspace(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_SPACE) != 0; }
inline uint32_t ASCII_ispunct(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_PUNC) != 0; }
inline uint32_t ASCII_isgraph(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_ISGRAPH) != 0; }
inline uint32_t ASCII_isprint(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_ISPRINT) != 0; }
inline uint32_t ASCII_iscntrl(uint32_t c) { return (ATOSE_to_ctype(c) & ATOSE_CTYPE_CONTROL) != 0; }
inline uint32_t ASCII_isascii(uint32_t c) { return c <= 0x7f; }

/*
	TO() routines
*/
inline uint8_t ASCII_tolower(uint32_t c) { return ATOSE_tolower_list[c]; }
inline uint8_t ASCII_toupper(uint32_t c) { return ATOSE_toupper_list[c]; }

#endif
