/*
	TEST_CIRCULAR_BUFFER.C
	----------------------
*/
#include <stdio.h>
#include "circular_buffer.h"

/*
	MAIN()
	------
*/
int main(void)
{
ATOSE_circular_buffer<char, 1024> buffer;
char *string = "qwertyuiop";
char *current;
char got;

for (current = string; *current != '\0'; current++)
	buffer.write(*current);

while (!buffer.is_empty())
	{
	got = buffer.read();
	printf("GOT:%d '%c'\n", got, got);
	}

return 0;
}