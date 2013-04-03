/*
	MALLOC.H
	--------
*/
#ifndef MALLOC_H_
#define MALLOC_H_

#ifndef NULL
	#define NULL 0
#endif

void KR_free(void *ap);
void *KR_malloc(unsigned nbytes);

#endif
