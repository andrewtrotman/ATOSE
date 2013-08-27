/*
	HELLO.C
	-------
	Hello world as an ATOSE process
*/
#include "../source-imx6/atose_api.h"

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_api api;

api.write('H');
api.write('e');
api.write('l');
api.write('l');
api.write('o');
api.write('\r');
api.write('\n');

for (;;);

return 0;
}
