/*
	IO_ANGEL.H
	----------
*/
#ifndef IO_ANGEL_H_
#define IO_ANGEL_H_

#include "io.h"
/*
	class ATOSE_IO_ANGEL
	--------------------
*/
class ATOSE_IO_angel : public ATOSE_IO
{
private:
	/*
		Angel SWI numbers
	*/
	#ifdef __thumb__
		static const int angel_swi_number = 0xAB;
	#else
		static const int angel_swi_number = 0x123456;
	#endif

	/*
		Angel function numbers
	*/
	static const int angel_swi_open  = 0x01;
	static const int angel_swi_write = 0x05;
	static const int angel_swi_read  = 0x06;

	/*
		file open modes (for angel_swi_open)
	*/
	static const int angel_swi_open_read = 0x00;
	static const int angel_swi_open_write = 0x04;

private:
	int stdin;				// standard input via the Angel ROM
	int stdout;				// standard output via the Angel ROM

private:
	int angel_swi(int function, void *parameter_block);
	int angel_open(const char *filename, int mode);


public:
	ATOSE_IO_angel();
	virtual ~ATOSE_IO_angel() {};

	virtual int read(char *buffer, int bytes);
	virtual int write(const char *buffer, int bytes);
} ;



#endif /* IO_ANGEL_H_ */
