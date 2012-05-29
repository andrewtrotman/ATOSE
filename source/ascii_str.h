/*
	ASCII_STR.H
	-----------
*/
#ifndef ASCII_STR_H_
#define ASCII_STR_H_

/*
	ASCII_STRLEN()
	--------------
*/
static inline int ASCII_strlen(const char *string)
{
const char *ch;

ch = string;
while (*ch != '\0')
	ch++;

return ch - string;
}

/*
	ASCII_ITOA()
	------------
*/
static inline char *ASCII_itoa(int value, char *destination, int base)
{
char sign, tmp;
char *into = destination;
char *from;

/*
	get the sign of the integer (is it -ve)
*/
if (value < 0)
	{
	sign = '-';
	value = -value;
	}
else
	sign = '\0';

/*
	do the digits in reverse order
*/
do
	{
	*into++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % base];
	value /= base;
	}
while (value > 0);

/*
	put the sign on
*/
if (sign != 0)
	*into++ = sign;

/*
	null terminate
*/
*into = '\0';

/*
	now reverse inplace
*/
into--;
for (from = destination; from < into; from++, into--)
	{
	tmp = *from;
	*from = *into;
	*into = tmp;
	}
return destination;
}

#endif /* ASCII_STR_H_ */
