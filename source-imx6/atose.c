/*
	ATOSE.C
	-------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose.h"
#include "stack.h"

/*
	ATOSE_ATOSE::ATOSE_ATOSE()
	--------------------------
*/
ATOSE_atose::ATOSE_atose() : debug(imx6q_serial_port), cpu(imx6q_cpu)
{
}

/*
	ATOSE_ATOSE::BOOT()
	-------------------
*/
void ATOSE_atose::boot(void)
{
/*
	No need to set up the stacks because that was done as part of the object creation
*/

}
