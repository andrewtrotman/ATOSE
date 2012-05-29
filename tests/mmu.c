/*
	MMU.C
	-----
*/
static const int MAX_SECTIONS = 4096;
unsigned long section_table[MAX_SECTIONS] __attribute__ ((aligned(0x4000)));


void mmu_init(void)
{
int section;

/*
	Set up a flat linear page table of 4096 pages of 1MB sections with "world" read/write permissions
*/
for (section = 0; section < MAX_SECTIONS; section++)
	section_table[section] = section << 20 | 0x0C12;

/*
	Enable the MMU
*/
asm("mrc	p15, 0, r0, c1, c0");			// read c1
asm("orr	r0, r0, #0x1000");				// turn on the 'S' bit
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("ldr	r0, =section_table");			// address of page table
asm("mcr	p15, 0, r0, c2, c0");			// write  c2

asm("mov	r0, #0xffffffff");				// domains (16 lots of %11) telling the MMU that we're the manager
asm("mcr	p15, 0, r0, c3, c0, 0");		// write c3

asm("mcr	p15, 0, r0, c7, c10, 4");		// write c7	 drain the cache write buffer

asm("mrc	p15, 0, r0, c1, c0, 0");		// read c1
asm("orr	r0, r0, #5");					// enable MMU, enable data cache
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("nop");								// noop so that the pipeline fills correctly
asm("nop");
asm("nop");
asm("nop" :::"r0");
}


void mmu_break(void)
{
int section;

/*
	Set up a flat linear page table of 4096 pages of 1MB sections with "world" read/write permissions
*/
for (section = 0; section < MAX_SECTIONS; section++)
	section_table[section] = 45 << 20 | 0x0C12;

/*
	Enable the MMU
*/
asm("mrc	p15, 0, r0, c1, c0");			// read c1
asm("orr	r0, r0, #0x1000");				// turn on the 'S' bit
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("ldr	r0, =section_table");			// address of page table
asm("mcr	p15, 0, r0, c2, c0");			// write  c2

asm("mov	r0, #0xffffffff");				// domains (16 lots of %11) telling the MMU that we're the manager
asm("mcr	p15, 0, r0, c3, c0, 0");		// write c3

asm("mcr	p15, 0, r0, c7, c10, 4");		// write c7	 drain the cache write buffer

asm("mrc	p15, 0, r0, c1, c0, 0");		// read c1
asm("orr	r0, r0, #5");					// enable MMU, enable data cache
asm("mcr	p15, 0, r0, c1, c0, 0");		// write c1

asm("nop");								// noop so that the pipeline fills correctly
asm("nop");
asm("nop");
asm("nop" :::"r0");
}


