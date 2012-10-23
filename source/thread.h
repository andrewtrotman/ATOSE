/*
	THREAD.H
	--------
*/
#ifndef THREAD_H_
#define THREAD_H_

#include "registers.h"

class ATOSE_process;

/*
	class ATOSE_THREAD
	------------------
*/
class ATOSE_thread
{
friend class ATOSE;
private:
	ATOSE_process *process;
	ATOSE_registers registers;

public:
	ATOSE_thread(ATOSE_process *process) { this->process = process; }
} ;

#endif /* THREAD_H_ */
