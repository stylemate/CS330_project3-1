#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "threads/thread.h"

struct frame
{
	struct hash_elem hash_elem;	/* Hash table element for frame */
	void *addr;					/* Virtual address */
	struct thread *thread;		/* Keeps track of the frames occupied by process */
};

void frame_init (void);

void *falloc_get_frame (enum palloc_flags);
void falloc_free_frame (void *);
