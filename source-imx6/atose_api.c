/*
	ATOSE_API.C
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#include "atose_api.h"
#include "ascii_str.h"

static const uint32_t LONGEST_ESC_SEQUENCE = 8;

/*
	Sequences the terminal sends to us
*/
static char ANSI_KEY_UP[]        = {0x1B, 0x5B, 0x41, 0x00};
static char ANSI_KEY_DOWN[]      = {0x1B, 0x5B, 0x42, 0x00};
static char ANSI_KEY_RIGHT[]     = {0x1B, 0x5B, 0x43, 0x00};
static char ANSI_KEY_LEFT[]      = {0x1B, 0x5B, 0x44, 0x00};
static char ANSI_KEY_HOME[]      = {0x1B, 0x5B, 0x31, 0x7E, 0x00};
static char ANSI_KEY_INSERT[]    = {0x1B, 0x5B, 0x32, 0x7E, 0x00};
static char ANSI_KEY_DELETE[]    = {0x1B, 0x5B, 0x33, 0x7E, 0x00};
static char ANSI_KEY_END[]       = {0x1B, 0x5B, 0x34, 0x7E, 0x00};
static char ANSI_KEY_PAGE_UP[]   = {0x1B, 0x5B, 0x35, 0x7E, 0x00};
static char ANSI_KEY_PAGE_DOWN[] = {0x1B, 0x5B, 0x36, 0x7E, 0x00};

/*
	Requests
*/
static char ANSI_GETXY[]      = {0x1B, '[', '6', 'n', '\0'};
static char ANSI_CLEAR_EOLN[] = {0x1B, '[', 'K','\0'};

/*
	We translate ESC sequences into key codes below
*/
static const uint32_t KEY_ENTER     = 0x0D;
static const uint32_t KEY_ESCAPE    = 0x1B;
static const uint32_t KEY_BACKSPACE = 0x7F;
static const uint32_t KEY_UP        = 0x00000100;
static const uint32_t KEY_DOWN      = 0x00000101;
static const uint32_t KEY_LEFT      = 0x00000102;
static const uint32_t KEY_RIGHT     = 0x00000103;
static const uint32_t KEY_HOME      = 0x00000104;
static const uint32_t KEY_INSERT    = 0x00000105;
static const uint32_t KEY_DELETE    = 0x00000106;
static const uint32_t KEY_END       = 0x00000107;
static const uint32_t KEY_PAGE_UP   = 0x00000108;
static const uint32_t KEY_PAGE_DOWN = 0x00000109;
static const uint32_t KEY_BAD       = 0xFFFFFFFF;						// this is an unknown ESC sequence

/*
	ATOSE_API::WRITELINE()
	----------------------
*/
char *ATOSE_api::writeline(const char *string)
{
char *start = (char *)string;

while (*string != '\0')
	{
	write(*string);
	string++;
	}

return start;
}

/*
	ATOSE_API::READ_KEY()
	---------------------
*/
uint32_t ATOSE_api::read_key(void)
{
long in_esc_mode = false;
char esc_sequence[LONGEST_ESC_SEQUENCE];
char *esc_key = esc_sequence;
uint32_t key;

do
	{
	key = read();

	if (in_esc_mode)
		{
		*esc_key++ = key;
		*esc_key = '\0';
		switch (esc_key - esc_sequence)
			{
			case 1: break;
			case 2: break;
			case 3:
				if (ASCII_strcmp(esc_sequence, ANSI_KEY_UP) == 0)
					return KEY_UP;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_DOWN) == 0)
					return KEY_DOWN;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_LEFT) == 0)
					return KEY_LEFT;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_RIGHT) == 0)
					return KEY_RIGHT;
				break;
			case 4:
				if (ASCII_strcmp(esc_sequence, ANSI_KEY_HOME) == 0)
					return KEY_HOME;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_INSERT) == 0)
					return KEY_INSERT;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_DELETE) == 0)
					return KEY_DELETE;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_END) == 0)
					return KEY_END;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_PAGE_UP) == 0)
					return KEY_PAGE_UP;
				else if (ASCII_strcmp(esc_sequence, ANSI_KEY_PAGE_DOWN) == 0)
					return KEY_PAGE_DOWN;
				break;
			default:
				return KEY_BAD;						// this is an unknown ESC sequence
			}
		}
	else if (key == KEY_ESCAPE)// && peek() != 0)
		{
		in_esc_mode = true;
		*esc_key++ = key;
		}
	else
		return key;
	}
while (1);

return 0;
}

/*
	ATOSE_API::GETXY()
	------------------
	returns 0 on success
*/
uint32_t ATOSE_api::getxy(uint32_t *x, uint32_t *y)
{
char string[13];			// the longest sequence assuming 3-digit numbers is 1+1+3+1+3+1 = 10.  We'll give it a bit of extra space just in case we see 4 digit numbers (and a '\0' for the end)
char *into, *semicolon;

/*
	Send the ANSI "report active position" request
*/
writeline(ANSI_GETXY);

/*
	This will return "<ESC>[<line>;<column>R" so we read until we get an 'R' within sizeof(string) characters back from the terminal
*/
for (into = string; (size_t)(into - string) < sizeof(string); into++)
	if ((*into = read()) == 'R')
		break;

*x = *y = 0;
if (*into != 'R')
	return 1;					// we failed to get a valid (x, y)

*into = '\0';

/*
	Apparently the terminal is not obliged to return a coordinate if it is 0... so we check our string is long enough
*/
if (into - string > 2)
	{
	*y = ASCII_atol(string + 2);			// <ESC>[<row>
	if ((semicolon = ASCII_strchr(string, ';')) != 0)
		*x = ASCII_atol(semicolon + 1);							// <ESC>[<row>;<column>
	}

return 0;
}

/*
	ATOSE_API::GOTOXY()
	-------------------
	return 0 on successful transmission to the terminal (which is not the same thing as success)
*/
uint32_t ATOSE_api::gotoxy(uint32_t x, uint32_t y)
{
char *into, buffer[13];			// assuming x and y are less then 3 digits each

if (x > 999 || y > 999)
	return 1;			// fail

/*
	Construct the escape sequence
*/
into = buffer;
*into++ = KEY_ESCAPE;
*into++ = '[';
ASCII_itoa(y, into, 10);
while (*into != '\0')
	into++;
*into++ = ';';
ASCII_itoa(x, into, 10);
while (*into != '\0')
	into++;
*into++ = 'H';
*into = '\0';

/*
	Transmit the escape sequence so as to go to (x,y)
*/
writeline(buffer);

return 0;
}

/*
	ATOSE_API::READLINE()
	---------------------
	returns NULL on fail and buffer on success
*/
char *ATOSE_api::readline(char *buffer, uint32_t length)
{
char *current_position = buffer;
char *end = buffer + length - 1;
uint32_t key, x, y;

getxy(&x, &y);

memset(buffer, 0, length);

if (length < 2)
	return NULL;
do
	{
	key = read_key();

	switch (key)
		{
		case KEY_ESCAPE:
			memset(buffer, 0, length);
			current_position = buffer;
			return NULL;		// fail
		case KEY_ENTER:
			*current_position = '\0';
			return buffer;		// success
		case KEY_BACKSPACE:
			if (current_position > buffer)
				{
				ASCII_strcpy(current_position - 1, current_position);
				current_position--;
				}
			break;
		case KEY_DELETE:
			ASCII_strcpy(current_position, current_position + 1);
			break;
		case KEY_LEFT:
			if (current_position > buffer)
				current_position--;
			break;
		case KEY_RIGHT:
			if (*current_position != '\0' && current_position < end)
				current_position++;
			break;
		case KEY_HOME:
			current_position = buffer;
			break;
		case KEY_END:
			while (*current_position != '\0' && current_position < end)
				current_position++;
			break;
		default:
			/*
				Ignore all keys that are outside the printable range
			*/
			if (ASCII_isprint(key))
				{
				*current_position = key;
				if (current_position < end)
					current_position++;
				}
			break;
		}
	gotoxy(x, y);
	writeline(ANSI_CLEAR_EOLN);
	writeline(buffer);
	gotoxy(x + (current_position - buffer), y);
	}
while (1);

return NULL;
}
