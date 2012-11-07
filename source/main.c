/*
	MAIN.C
	------
	Entry point to ATOSE
*/
#include <stdint.h>
#include "atose.h"
#include "api_atose.h"
#include "../examples/hello.elf.c"

#ifdef IMX233
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regspower.h"
	#include "../systems/imx-bootlets-src-10.05.02/mach-mx23/includes/registers/regsuartdbg.h"
#endif



int ATOSE_main(void);
int main(void);

extern "C"
{
	/*
		C_ENTRY()
		---------
	*/
	void c_entry() 
	{
	#ifdef FourARM
		/*
			Magic to get around the brownout problem in the FourARM
		*/
		HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 0;
	#endif
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
}

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE os;

os.enable(ATOSE_main);
return 0;
}

/*
	ATOSE_MAIN()
	------------
	This is where ATOSE ends up after the OS has been initialised and we've dropped into user space
*/
#ifdef NEVER
	int ATOSE_main(void)
	{
	ATOSE_API_ATOSE api;

	api.io.hex();

	for (;;)
		{
		uint8_t got;

	#ifdef QEMU
		if (api.keyboard.read_byte(&got))
			api.io << "KBM: " << (long)got << ATOSE_IO::eoln;
		if (api.mouse.read_byte(&got))
			api.io << "MOU: " << (long)got << ATOSE_IO::eoln;
	#endif
		if (api.io.read_byte(&got))
			api.io << "COM: " << (long)got << " '" << got << "'" << ATOSE_IO::eoln;
		}

	return 0;
	}
#endif

/*
	ATOSE_MAIN()
	------------
	This is where ATOSE ends up after the OS has been initialised and we've dropped into user space
*/
int ATOSE_main(void)
{
ATOSE_API_ATOSE api;
uint64_t loop, MAX = 100;

api.io.write((uint8_t *)"ATOSE\r\n", 7);

//api.io << "ATOSE" << ATOSE_IO::eoln;

for (loop = 0; loop < MAX; loop++)
	api.io << ":";
 
api.process_manager.write_block(ATOSE_elf_hello, ATOSE_elf_hello_size);

for (;;)
	api.io << ":";

return 0;
}
