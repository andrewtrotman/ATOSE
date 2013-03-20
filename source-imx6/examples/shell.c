/*
	SHELL.C
	-------
	Really simple shell used to get ATOSE up and running on the SABRE Lite board
*/
#include <stdio.h>
#include "../ascii_str.h"
#include "../atose_api.h"

#define MAX_COMMAND_LENGTH 1024

/*
	MAIN()
	------
*/
int main(void)
{
char command[MAX_COMMAND_LENGTH];

while (fgets(command, sizeof(command), stdin) != NULL)
	{
	puts(command);
	}
}
