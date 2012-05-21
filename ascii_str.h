/*
	ASCII_STR.H
	-----------
*/
#ifndef ASCII_STR_H_
#define ASCII_STR_H_

static inline int ASCII_strlen(const char *string)
{
const char *ch;

ch = string;
while (*ch != '\0')
	ch++;

return ch - string;
}

#endif /* ASCII_STR_H_ */
