/*
	IDLE.C
	------
	This is the ATOSE IDLE process.
*/
#include "idle.h"
#include "atose_api.h"
#include "examples/hello1.elf.c"
#include "examples/hello2.elf.c"

/*
	IDLE()
	------
*/
uint32_t idle(void)
{
ATOSE_api api;

api.write('[');
api.write('I');
api.write('D');
api.write('L');
api.write('E');
api.write(']');

api.spawn(hello1_elf, hello1_elf_size);
api.spawn(hello2_elf, hello2_elf_size);

while (1)
	api.write('A');

return 0;
}
