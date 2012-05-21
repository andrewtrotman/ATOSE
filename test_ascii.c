/*
	TEST_ASCII.C
	------------
*/
#include <string.h>
#include <stdio.h>
#include "ascii_str.h"

char into[1024];

int main(void)
{
char *string = "qwerty";
int len;

if (ASCII_strlen(string) != strlen(string))
	printf("ASCII_strlen() failure\n");

ASCII_itoa(1234567890, into, 10);
	if (strcmp("1234567890", into) != 0)
		printf("ASCII_itoa() failure (answer was:%s)\n", into);
}
