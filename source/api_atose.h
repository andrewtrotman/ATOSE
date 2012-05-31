/*
	API_ATOSE.H
	-----------
*/
#ifndef API_ATOSE_H_
#define API_ATOSE_H_

#define ATOSE_SWI 0x6174
/*
	class ATOSE_API_MOUSE
	---------------------
*/
class ATOSE_API_mouse
{
public:
	uint32_t read_byte(uint8_t *got)
	{
	uint32_t answer;
	uint32_t function = 1;
	uint32_t byte;

	asm volatile 
		(
		"mov r0, %[function]; "
		"swi %[ATOSE_swi]; "
		"mov %[answer], r1; "
		"mov %[byte], r0; "
		: [answer]"=r" (answer), [byte]"=r" (byte)
		: [function]"r" (function), [ATOSE_swi]"i" (ATOSE_SWI)
		: "r0", "r1"
		);
	*got = (uint8_t)byte;
	return answer;
	}
} ;

/*
	class ATOSE_API_ATOSE
	---------------------
*/
class ATOSE_API_ATOSE
{
public:
	ATOSE_API_mouse mouse;
} ;


#endif /* API_ATOSE_H_ */
