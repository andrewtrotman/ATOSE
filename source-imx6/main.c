/*
	MAIN.C
	------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD

	This module contains the main entry point, it also houses (on its stack) the
	object that is the operating system
*/
#include "atose.h"

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_atose atose;		// create and initialise everything
atose.reset();			// now pass control to the OS

while (1)
	;	// do nothing

return 0;				// like as if this is ever going to happen!
}

