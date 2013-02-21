/*
	SEMAPHORE.H
	-----------
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
	int32_t value;

public:
	ATOSE_semaphore *next;		// for the purpose of free chains

private:
	void wake_one(void);
	void sleep_one(ATOSE_registers *registers);

public:
	ATOSE_semaphore() {}

	void clear(void);
	void wait(ATOSE_registers *registers);
	void signal(void);
} ;

#endif
