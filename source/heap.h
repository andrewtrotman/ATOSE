/*
	HEAP.H
	------
*/
#ifndef HEAP_H_
#define HEAP_H_

/*
	The size of the kernel heap can be configured in the makefile, but defaults here
*/
#ifndef ATOSE_KERNEL_HEAP_SIZE
	#define ATOSE_KERNEL_HEAP_SIZE 128
#endif

/*
	class ATOSE_HEAP
	----------------
*/
class ATOSE_heap
{
private:
	static const size_t heap_size = ATOSE_KERNEL_HEAP_SIZE; 
	static unsigned char heap[heap_size];

private:
	long long used;
	void *bottom_of_heap;

public:
	ATOSE_heap();
	virtual ~ATOSE_heap() {}

	void *alloc(size_t bytes);
	void *free(void *memory);
} ;

#endif /* HEAP_H_ */

