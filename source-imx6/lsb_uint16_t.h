/*
	LSB_UINT16_T.H
	--------------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef LSB_UINT16_T_H_
#define LSB_UINT16_T_H_

/*
	class ATOSE_LSB_UINT16_T
	------------------------
*/
class ATOSE_lsb_uint16_t
{
private:
	union
		{
		uint16_t value;
		struct
			{
			uint8_t byte_2;			// LSB
			uint8_t byte_1;			// MSB
			} __attribute__ ((packed));
		} __attribute__ ((packed));

public:
	operator uint16_t() { return (byte_1 << 8) | byte_2; }
	ATOSE_lsb_uint16_t *operator =(uint16_t value) { byte_1 = (value >> 8) & 0xFF; byte_2 = value & 0xFF; return this; }
} __attribute__ ((packed));

#endif
