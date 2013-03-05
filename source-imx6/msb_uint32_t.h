/*
	MSB_UINT32_T.H
	--------------
*/
#ifndef MSB_UINT32_T_H_
#define MSB_UINT32_T_H_

/*
	class ATOSE_MSB_UINT32_T
	------------------------
*/
class ATOSE_msb_uint32_t
{
private:
	union
		{
		uint32_t value;
		struct
			{
			uint8_t byte_1;			// MSB
			uint8_t byte_2;
			uint8_t byte_3;
			uint8_t byte_4;			// LSB
			} __attribute__ ((packed));
		} __attribute__ ((packed));

public:
	operator uint64_t() { return (uint64_t)operator uint32_t(); }
	operator uint32_t() { return (byte_1 << 24) | (byte_2 << 16) | (byte_3 << 8) | byte_4; }
	operator uint16_t() { return (byte_3 << 8) | byte_4; }
	operator uint8_t()  { return byte_4; }

	operator int64_t() { return (int64_t)operator uint64_t(); }
	operator int32_t() { return (int32_t)operator uint32_t(); }
	operator int16_t() { return (int16_t)operator uint16_t(); }
	operator int8_t()  { return (int8_t)operator uint8_t(); }

	ATOSE_msb_uint32_t *operator =(uint32_t value) { byte_1 = (value >> 24) & 0xFF; byte_2 = (value >> 16) & 0xFF; byte_3 = (value >> 8) & 0xFF; byte_4 = (value >> 0) & 0xFF; return this; }
	ATOSE_msb_uint32_t *operator =(uint16_t value) { byte_1 = byte_2 = 0; byte_3 = (value >> 8) & 0xFF; byte_4 = (value >> 0) & 0xFF; return this; }
	ATOSE_msb_uint32_t *operator =(uint8_t value)  { byte_1 = byte_2 = byte_3 = 0; byte_4 = (value >> 0) & 0xFF; return this; }

	ATOSE_msb_uint32_t *operator =(int32_t value) { return operator=((uint32_t)value); }
	ATOSE_msb_uint32_t *operator =(int16_t value) { return operator=((uint16_t)value); }
	ATOSE_msb_uint32_t *operator =(int8_t value)  { return operator=((uint8_t)value); }
} __attribute__ ((packed));

#endif
