/*
	SEMAPHORE.H
	-----------
	Copyright (c) 2013 Andrew Trotman
	Licensed BSD
*/
#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <stdint.h>

#include "process.h"
#include "registers.h"

/*
	class ATOSE_SEMAPHORE
	---------------------
*/
class ATOSE_semaphore
{
private:
	ATOSE_process *sleepers;
	volatile int32_t value;

public:
	ATOSE_semaphore *next;		// for the purpose of free chains

private:
	void wake_one(void);
	void sleep_one(void);

public:
	ATOSE_semaphore() {}

	void clear(void);
	void wait(void);
	void signal(void);
} ;

#endif
