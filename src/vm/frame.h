#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <hash.h>
#include "threads/palloc.h"
#include "threads/thread.h"

struct frame
{
	struct hash_elem hash_elem;	/* Hash table element for frame */
	void *addr;					/* Virtual address */
	struct thread *thread;		/* Keeps track of the frames occupied by process */
	void *upage;				/* Keeps track of corresponding page */
};

void frame_init (void);

void *falloc_get_frame (void *, enum palloc_flags);
void falloc_free_frame (void *);

#endif
