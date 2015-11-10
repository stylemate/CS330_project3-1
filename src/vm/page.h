#include <hash.h>
#include "threads/synch.h"
#include "threads/palloc.h"
#include "threads/thread.h"

struct sup_page
{
	struct hash_elem hash_elem;	/* Hash table element for page */
	void *addr;					/* Virtual address */
	enum page_location location;/* Where is the page? */
};

enum page_location
{
	FILE_SYSTEM,
	SWAP_DISK,
	ZERO
}