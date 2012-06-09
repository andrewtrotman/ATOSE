/*
	MAIN.C
	------
	Entry point to ATOSE
*/
#include <stdint.h>
#include "atose.h"
#include "api_atose.h"

extern "C" { int main2(void); }

extern "C"
{
	/*
		C_ENTRY()
		---------
	*/
	void c_entry() 
	{
	/*
		to start with we'll make sure the interrupt vectors are correct
	*/
	extern uint32_t ATOSE_vectors_start;
	extern uint32_t ATOSE_vectors_end;
	uint32_t *vectors_src = &ATOSE_vectors_start;
	uint32_t *vectors_dst = (uint32_t *)0;

	while(vectors_src < &ATOSE_vectors_end)
		*vectors_dst++ = *vectors_src++;
	/*
		then we'll call all the constructors
	*/

	extern void (*start_ctors)();
	extern void (*end_ctors)();
	void (**constructor)() = &start_ctors;

	while (constructor < &end_ctors)
		{
		(*constructor)();
		constructor++;
		}

	/*
		Call main
	*/
	extern int main(void);
	main();
	}

	/*
		Stubs necessary to get the compiler to work
	*/
	/*
		ABORT()
		-------
	*/
	void abort(void)
	{
	}

	/*
		_SBRK_R ()
		----------
	*/
	void *_sbrk_r(int inc)
	{
	return 0;
	}
}

/*
	MAIN()
	------
*/
uint8_t st_user_stack[8192];

uint32_t user_stack = (uint32_t)(st_user_stack + 4096);
uint32_t tos1;
uint32_t tos2;

int main(void)
{
ATOSE os;

asm volatile ( "mov %0, sp;" : "=r"(tos1));

/*
	Switch to user mode 
*/
asm volatile
	(
	"mrs r0, CPSR;"
	"bic r0, r0, #0x01F;"
	"orr r0, r0, #0x10;"
	"msr CPSR_cxsf, r0;"
	:
	:
	: "r0"
	);

asm volatile ( "mov %0, sp;" : "=r"(tos2));

/*
asm volatile
	(
	"mrs r0, CPSR;"
	"bic r0, r0, #0x01F;"
	"orr r0, r0, #0x10;"
	"msr CPSR_cxsf, r0;"
	"mov sp, %[user_stack];"
	:
	: [user_stack]"r"(user_stack)
	: "r0"
	);
*/

return main2();
}

int main2(void)
{

ATOSE_API_ATOSE api;

api.io << "system:" << (long)tos1 << " user:" << (long)tos2 << ATOSE_IO::eoln;
api.io << "kbd_id1:" << api.keyboard.id_object << ATOSE_IO::eoln;
api.io << "Mouse_id1:" << api.mouse.id_object << ATOSE_IO::eoln;
uint32_t ans;

api.io << "Mouse_id2:" << api.mouse.id_object << ATOSE_IO::eoln;
api.io.hex();
api.io << "Mouse_id2a:" << api.mouse.id_object << ATOSE_IO::eoln;
api.io.decimal();
api.io << "Mouse_id2b:" << api.mouse.id_object << ATOSE_IO::eoln;
api.io.hex();
api.io << "Mouse_id2c:" << api.mouse.id_object << ATOSE_IO::eoln;


extern uint32_t ATOSE_top_of_memory;

api.io << "Mouse_id3:" << api.mouse.id_object << ATOSE_IO::eoln;
api.keyboard.write_byte(0xFF);
api.io << "Mouse_id4:" << api.mouse.id_object << ATOSE_IO::eoln;
api.mouse.write_byte(0xFF);
api.mouse.write_byte(0xF4);
api.io << "Mouse_id5:" << api.mouse.id_object << ATOSE_IO::eoln;

for (;;)
	{
	uint8_t got;

	if (api.keyboard.read_byte(&got))
		api.io << "KBM: " << (long)got << ATOSE_IO::eoln;
	if (api.mouse.read_byte(&got))
		api.io << "MOU: " << (long)got << ATOSE_IO::eoln;
	if (api.io.read_byte(&got))
		{
		api.io << "COM: " << (long)got << ATOSE_IO::eoln;
//		ans = api.mouse.read_byte(&got);
//		api.io << "API got: " << (long)got << " ans:" << (long)ans << ATOSE_IO::eoln;
		}
	}

api.io << "Mouse_id6:" << api.mouse.id_object << ATOSE_IO::eoln;

return 0;
}
