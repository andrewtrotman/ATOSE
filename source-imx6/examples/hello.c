/*
	HELLO.C
	-------
	Hello world as an ATOSE process
*/

#include "../atose_api.h"

#ifndef CHARACTER
	#define CHARACTER 'B'
#endif

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_api api;

for (;;)
	api.write(CHARACTER);

return 0;
}
