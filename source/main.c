/*
	MAIN.C
	------
	Entry point to ATOSE
*/
#include <stdint.h>
#include "atose.h"
#include "api_atose.h"

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
		Now enter main
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
int main(void)
{
ATOSE os;
ATOSE_API_ATOSE api;
uint32_t ans;
uint8_t byte;

os.io.hex();

extern uint32_t ATOSE_top_of_memory;

if (&ATOSE_top_of_memory == 0)
	os.io << "Top of Memory:" << "zero" << ATOSE_IO::eoln;
else
	os.io << "Top of Memory:" << (long)&ATOSE_top_of_memory << ATOSE_IO::eoln;

os.keyboard.write_byte(0xFF);
os.mouse.write_byte(0xFF);
os.mouse.write_byte(0xF4);
for (;;)
	{
	char got;

	if (os.keyboard.read_byte(&got))
		os.io << "KBM: " << (long)got << ATOSE_IO::eoln;
	if (os.mouse.read_byte(&got))
		os.io << "MOU: " << (long)got << ATOSE_IO::eoln;
	if (os.io.read_byte(&got))
		{
		os.io << "COM: " << (long)got << ATOSE_IO::eoln;
		ans = api.mouse.read_byte(&byte);
		os.io << "API got: " << (long)byte << " ans:" << (long)ans << ATOSE_IO::eoln;
		}
	}

return 0;
}
