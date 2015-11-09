#include <hash.h>
#include "threads/synch.h"

struct frame
{
	struct hash_elem hash_elem; /* Hash table element for frame */
	void *addr;					/* Virtual address */
};

void frame_init (void);