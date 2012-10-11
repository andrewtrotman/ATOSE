/*
	PROCESS.H
	---------
*/
#ifndef PROCESS_H_
#define PROCESS_H_

/*
	class ATOSE_PROCESS
	-------------------
*/
class ATOSE_process
{
private:
	ATOSE_address_space address_space;
	ATOSE_thread execution_path;

public:
	ATOSE_process() : address_space(mmu, process_id) : execution_path(process_id) {}
} ;

#endif /* PROCESS_H_ */
