#include <string.h>
#include "../source-imx6/atose_api.h"

unsigned char buffer[1024*1024];

int main(void)
{
ATOSE_api api;
int x;

for (x = 0; x < 0xFF; x++)
	{
	__builtin_memset(buffer, 0, sizeof(buffer));
	api.write('.');
	}

for(;;);
}

