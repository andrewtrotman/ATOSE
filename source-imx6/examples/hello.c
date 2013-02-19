/*
	HELLO.C
	-------
	Hello world as an ATOSE process
*/

#include "../atose_api.h"


/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_api api;

for (;;)
	api.write('B');

return 0;
}
