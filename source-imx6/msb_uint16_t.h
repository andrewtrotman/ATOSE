/*
	MSB_UINT16_T.H
	--------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef MSB_UINT16_T_H_
#define MSB_UINT16_T_H_

/*
	class ATOSE_MSB_UINT16_T
	------------------------
*/
class ATOSE_msb_uint16_t
{
private:
	union
		{
		uint16_t value;
		struct
			{
			uint8_t byte_1;			// MSB
			uint8_t byte_2;			// LSB
			} __attribute__ ((packed));
		} __attribute__ ((packed));

public:
	operator uint64_t() { return (uint64_t)operator uint16_t(); }
	operator uint32_t() { return (uint32_t)operator uint16_t(); }
	operator uint16_t() { return (byte_1 << 8) | byte_2; }
	operator uint8_t()  { return byte_2; }

	operator int64_t() { return (int64_t)operator uint64_t(); }
	operator int32_t() { return (int32_t)operator uint32_t(); }
	operator int16_t() { return (int16_t)operator uint16_t(); }
	operator int8_t()  { return (int8_t)operator uint8_t(); }

	ATOSE_msb_uint16_t *operator =(uint16_t value) { byte_1 = (value >> 8) & 0xFF; byte_2 = value & 0xFF; return this; }
	ATOSE_msb_uint16_t *operator =(uint8_t value)  { byte_1 = 0; byte_2 = value & 0xFF; return this; }

	ATOSE_msb_uint16_t *operator =(int16_t value) { return operator=((uint16_t)value); }
	ATOSE_msb_uint16_t *operator =(int8_t value)  { return operator=((uint8_t)value); }
} __attribute__ ((packed));

#endif
