/*
	MSB_UINT64_T.H
	--------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef MSB_UINT64_T_H_
#define MSB_UINT64_T_H_

/*
	class ATOSE_MSB_UINT64_T
	------------------------
*/
class ATOSE_msb_uint64_t
{
private:
	union
		{
		uint64_t value;
		struct
			{
			uint8_t byte_1;			// MSB
			uint8_t byte_2;
			uint8_t byte_3;
			uint8_t byte_4;
			uint8_t byte_5;
			uint8_t byte_6;
			uint8_t byte_7;
			uint8_t byte_8;			// LSB
			} __attribute__ ((packed));
		} __attribute__ ((packed));

public:
	operator uint64_t() { return ((uint64_t)byte_1 << 56) | ((uint64_t)byte_2 << 48) | ((uint64_t)byte_3 << 40) | ((uint64_t)byte_4 << 32) | ((uint64_t)byte_5 << 24) | ((uint64_t)byte_6 << 16) | ((uint64_t)byte_7 << 8) | (uint64_t)byte_8; }
	ATOSE_msb_uint64_t *operator =(uint64_t value) { byte_1 = (value >> 56) & 0xFF; byte_2 = (value >> 48) & 0xFF; byte_3 = (value >> 40) & 0xFF; byte_4 = (value >> 32) & 0xFF; byte_5 = (value >> 24) & 0xFF; byte_6 = (value >> 16) & 0xFF; byte_7 = (value >>  8) & 0xFF; byte_8 = (value >>  0) & 0xFF; return this; }
} __attribute__ ((packed));

#endif
