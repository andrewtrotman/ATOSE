/*
	HELLO.C
	-------
	Hello world as an ATOSE process
*/

#include "../source/api_atose.h"


/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_API_ATOSE api;

api.io << "Hello World" << ATOSE_IO::eoln;

for (;;)
	api.io << "?";

return 0;
}
