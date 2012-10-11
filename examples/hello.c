/*
	HELLO.C
	-------
	Hello world as an ATOSE process
*/

#include "api_atose.h"

ATOSE_API_ATOSE api;

/*
	MAIN()
	------
*/
int main(void)
{
api.io << "Hello World" << ATOSE_IO::eoln;

return 0;
}