/*
	IO_ANGEL.C
	----------
	This file implements input and output by performing SWI calls to the Angel Monitor ROM.
	Angel appears to be implemented as part of the GNU simulator, as part of QEMU, and
	on many hardware boards too.
*/
#include "ascii_str.h"
#include "io_angel.h"

/*
	ATOSE_IO_ANGEL::INIT()
	----------------------
*/
void ATOSE_IO_angel::init(void)
{
ATOSE_IO::init();
stdin = angel_open(":tt", angel_swi_open_read);
stdout = angel_open(":tt", angel_swi_open_write);
}

/*
	ATOSE_IO_ANGEL::ANGEL_SWI()
	---------------------------
	Given a param block, make the given call to Angel through the SWI interface
*/
inline int ATOSE_IO_angel::angel_swi(int function, void *parameter_block)
{
int value;

/*
	This calls through to the Angel Monitor ROM to perform the action.
	This code destroys r0, r1, and Angel will destroy lr in supervisor mode
*/
asm volatile
	(
	"mov r0, %1;"
	"mov r1, %2;"
	"swi %a3; "
	"mov %0, r0"
	: "=r" (value) 
	: "r" (function), "r" (parameter_block), "i" (angel_swi_number) 
	: "r0", "r1", "lr");

return value;
}

/*
	ATOSE_IO_ANGEL::ANGEL_OPEN()
	----------------------------
*/
int ATOSE_IO_angel::angel_open(const char *filename, int mode)
{
int param_block[3];

param_block[0] = (int)filename;
param_block[1] = mode;
param_block[2] = ASCII_strlen(filename);

return angel_swi(angel_swi_open, param_block);
}

/*
	ATOSE_IO_ANGEL::READ()
	----------------------
	read a string of bytes from stdin
	Returns the number of bytes read (or 0 on read past EOF)
*/
uint32_t ATOSE_IO_angel::read(uint8_t *buffer, uint32_t  bytes)
{
int param_block[3];
uint32_t answer;

param_block[0] = stdin;
param_block[1] = (int)buffer;
param_block[2] = (int)bytes;

answer = angel_swi(angel_swi_read, param_block);

/*
	Check return codes
*/
if (answer == 0)
	return bytes;		// success
else if (answer < bytes)
	return answer;		// number of bytes read
else
	return 0;			// read past EOF
}

/*
	ATOSE_IO_ANGEL::WRITE()
	-----------------------
	write a byte buffer to stdout
	Returns the numner of bytes written (which might be zero)
*/
uint32_t  ATOSE_IO_angel::write(const uint8_t *buffer, uint32_t  bytes)
{
int param_block[3];
uint32_t answer;

param_block[0] = stdout;
param_block[1] = (int)buffer;
param_block[2] = (int)bytes;

answer = angel_swi(angel_swi_write, param_block);

/*
	Check return codes
*/
if (answer == 0)
	return bytes;				// success
else
	return bytes - answer;		// number of bytes written
}
