/*
	PROCESS.H
	---------
*/
#ifndef PROCESS_H_
#define PROCESS_H_

#include "address_space.h"
#include "thread.h"

class ATOSE_mmu;

/*
	class ATOSE_PROCESS
	-------------------
*/
class ATOSE_process
{
public:
	ATOSE_process *next;
	ATOSE_address_space address_space;
	ATOSE_thread execution_path;
	size_t entry_point;

public:
	ATOSE_process(ATOSE_mmu *mmu) : address_space(mmu), execution_path(this) {}
} ;

#endif /* PROCESS_H_ */
