/*
	ATOSE_API.H
	-----------
*/

#ifndef ATOSE_API_H_
#define ATOSE_API_H_

#include <stdint.h>

/*
	The SWI number used for standard system calls
*/
#define ATOSE_SWI 0x4154

/*
	List of ATOSE operating system entry points
*/
enum
	{
	ATOSE_WRITE_BYTE,
	ATOSE_SPAWN,
	ATOSE_END_OF_METHODS
	} ;

/*
	class ATOSE_API
	---------------
*/
class ATOSE_api
{
public:
	ATOSE_api() {}

	uint32_t SYSTEM_CALL(uint32_t function_id, uint32_t parameter_1 = 0, uint32_t parameter_2 = 0)
	{
	uint32_t answer;

	asm volatile
		(
		"mov r0, %[function_id];"
		"mov r1, %[parameter_1];"
		"mov r2, %[parameter_2];"
		"swi %[ATOSE_swi];"
		"mov %[answer], r0;"
		: [answer]"=r" (answer)
		: [function_id]"r"(function_id), [parameter_1]"r"(parameter_1), [parameter_2]"r"(parameter_2), [ATOSE_swi]"i"(ATOSE_SWI)
		: "r0", "r1", "r2"
		);
	return answer;
	}

	uint32_t write(uint8_t byte) { return SYSTEM_CALL(ATOSE_WRITE_BYTE, byte); }
	uint32_t spawn(unsigned char *elf, uint32_t bytes) { return SYSTEM_CALL(ATOSE_SPAWN, (uint32_t)elf, bytes); }
} ;

#endif
