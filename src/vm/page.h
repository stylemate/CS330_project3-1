#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "threads/thread.h"

enum page_location
{
	FILE_SYSTEM,
	SWAP_DISK,
	ZERO
};

struct sup_page
{
	struct hash_elem hash_elem;	/* Hash table element for page */
	void *addr;					/* Virtual address */
	enum page_location location;/* Where is the page? */
};

#endif
