/*
	MAIN.C
	------
*/
#include "io_angel.h"

extern void mmu_init(void);
extern void mmu_break(void);
extern void ATOSE_default_isr(void);

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_IO *io = new ATOSE_IO_angel;
char buffer[1024];

io->puts("Enter");
mmu_init();
io->puts("Done MMU init");
mmu_break();
io->puts("Done MMU break");

return 0;
}

