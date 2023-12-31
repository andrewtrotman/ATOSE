/*
	MALLOC.C
	--------
	The K&R Malloc source code directly out of the book (and reformatted to look pretty)
*/
#include "atose_api.h"
#include "malloc.h"

#define NALLOC 1024 /* minimum #units to request */

typedef long Align; /* for alignment to long boundary */

/*
	union HEADER
	------------
*/
union header
{ /* block header */
struct
	{
	union header *ptr; /* next block if on free list */
	unsigned size; /* size of this block */
	} s;
Align x; /* force alignment of blocks */
};

typedef union header Header;

static Header base; /* empty list to get started */
static Header *freep = NULL; /* start of free list */

/*
	KR_FREE()
	---------
	put block ap in free list
*/
void KR_free(void *ap)
{
Header *bp, *p;

bp = (Header *)ap - 1; /* point to block header */
for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
	if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
		break; /* freed block at start or end of arena */

if (bp + bp->s.size == p->s.ptr)
	{ /* join to upper nbr */
	bp->s.size += p->s.ptr->s.size;
	bp->s.ptr = p->s.ptr->s.ptr;
	}
else
	bp->s.ptr = p->s.ptr;

if (p + p->s.size == bp)
	{ /* join to lower nbr */
	p->s.size += bp->s.size;
	p->s.ptr = bp->s.ptr;
	}
else
	p->s.ptr = bp;

freep = p;
}

/*
	SBRK()
	------
*/
char *sbrk(int bytes)
{
return (char *)ATOSE_api::set_heap_break((uint32_t)bytes);
}

/*
	MORECORE()
	----------
	Ask system for more memory
*/
static Header *morecore(unsigned nu)
{
char *cp;
Header *up;

if (nu < NALLOC)
	nu = NALLOC;

cp = sbrk(nu * sizeof(Header));

if (cp == NULL) /* no space at all */
	return NULL;

up = (Header *) cp;
up->s.size = nu;
KR_free((void *)(up + 1));

return freep;
}

/*
	KR_MALLOC()
	-----------
	General-purpose storage allocator
*/
void *KR_malloc(unsigned nbytes)
{
Header *p, *prevp;
unsigned nunits;

nunits = (nbytes + sizeof(Header) - 1) / sizeof(header) + 1;

if ((prevp = freep) == NULL)
	{/* No free list yet */
	base.s.ptr = freep = prevp = &base;
	base.s.size = 0;
	}

for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr)
	{
	if (p->s.size >= nunits)
		{ /* Big enough */
		if (p->s.size == nunits)	/* exactly */
			prevp->s.ptr = p->s.ptr;
		else
			{ /* allocate tail end */
			p->s.size -= nunits;
			p += p->s.size;
			p->s.size = nunits;
			}
		freep = prevp;
		return (void *)(p + 1);
		}

	if (p == freep) /* wrapped around free list */
		if ((p = morecore(nunits)) == NULL)
			return NULL; /* none left */
	}

return NULL;
}

