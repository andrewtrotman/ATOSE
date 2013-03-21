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
	operator uint32_t() { return (byte_5 << 24) | (byte_6 << 16) | (byte_7 << 8) | byte_8; }
	operator uint16_t() { return (byte_7 << 8) | byte_8; }
	operator uint8_t()  { return byte_8; }

	operator int64_t() { return (int64_t)operator uint64_t(); }
	operator int32_t() { return (int32_t)operator uint32_t(); }
	operator int16_t() { return (int16_t)operator uint16_t(); }
	operator int8_t()  { return (int8_t)operator uint8_t(); }

	ATOSE_msb_uint64_t *operator =(uint64_t value) { byte_1 = (value >> 56) & 0xFF; byte_2 = (value >> 48) & 0xFF; byte_3 = (value >> 40) & 0xFF; byte_4 = (value >> 32) & 0xFF; byte_5 = (value >> 24) & 0xFF; byte_6 = (value >> 16) & 0xFF; byte_7 = (value >>  8) & 0xFF; byte_8 = (value >>  0) & 0xFF; return this; }
	ATOSE_msb_uint64_t *operator =(uint32_t value) { byte_1 = byte_2 = byte_3 = byte_4 = 0; byte_5 = (value >> 24) & 0xFF; byte_6 = (value >> 16) & 0xFF; byte_7 = (value >>  8) & 0xFF; byte_8 = (value >>  0) & 0xFF; return this; }
	ATOSE_msb_uint64_t *operator =(uint16_t value) { byte_1 = byte_2 = byte_3 = byte_4 = byte_5 = byte_6 = 0; byte_7 = (value >> 8) & 0xFF; byte_8 = (value >> 0) & 0xFF; return this; }
	ATOSE_msb_uint64_t *operator =(uint8_t value)  { byte_1 = byte_2 = byte_3 = byte_4 = byte_5 = byte_6 = byte_7 = 0; byte_8 = (value >> 0) & 0xFF; return this; }

	ATOSE_msb_uint64_t *operator =(int64_t value) { return operator=((uint64_t)value); }
	ATOSE_msb_uint64_t *operator =(int32_t value) { return operator=((uint32_t)value); }
	ATOSE_msb_uint64_t *operator =(int16_t value) { return operator=((uint16_t)value); }
	ATOSE_msb_uint64_t *operator =(int8_t value)  { return operator=((uint8_t)value); }
} __attribute__ ((packed));

#endif
