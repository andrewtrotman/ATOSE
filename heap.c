/*
	HEAP.C
	------
*/
#include "heap.h"

unsigned char ATOSE_heap::heap[heap_size];

/*
	ATOSE_HEAP::ATOSE_HEAP()
	------------------------
*/
ATOSE_heap::ATOSE_heap()
{
bottom_of_heap = heap;
used = 0;
}

/*
	ATOSE_HEAP::ALLOC()
	-------------------
*/
void *ATOSE_heap::alloc(size_t bytes)
{
void *answer;

answer = bottom_of_heap;
bottom_of_heap = ((unsigned char *)bottom_of_heap) + bytes;
used += bytes;

return answer;
}

/*
	ATOSE_HEAP::FREE()
	------------------
*/
void *ATOSE_heap::free(void *memory)
{
/*
	At present, just crap out!
*/
}

void operator delete(void*)
{
}

