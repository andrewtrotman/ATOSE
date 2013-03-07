/*
	LSB_UINT32_T.H
	--------------
*/
#ifndef LSB_UINT32_T_H_
#define LSB_UINT32_T_H_

/*
	class ATOSE_LSB_UINT32_T
	------------------------
*/
class ATOSE_lsb_uint32_t
{
private:
	union
		{
		uint32_t value;
		struct
			{
			uint8_t byte_4;			// LSB
			uint8_t byte_3;
			uint8_t byte_2;
			uint8_t byte_1;			// MSB
			} __attribute__ ((packed));
		} __attribute__ ((packed));

public:
	operator uint32_t() { return (byte_1 << 24) | (byte_2 << 16) | (byte_3 << 8) | byte_4; }
	ATOSE_lsb_uint32_t *operator =(uint32_t value) { byte_1 = (value >> 24) & 0xFF; byte_2 = (value >> 16) & 0xFF; byte_3 = (value >> 8) & 0xFF; byte_4 = (value >> 0) & 0xFF; return this; }
} __attribute__ ((packed));

#endif
