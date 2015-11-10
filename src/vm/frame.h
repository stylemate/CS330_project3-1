#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"

struct frame
{
	struct hash_elem hash_elem;	/* Hash table element for frame */
	void *addr;					/* Virtual address */
};

void frame_init (void);

void *falloc_get_frame (enum palloc_flags);
void falloc_free_frame (void *);
