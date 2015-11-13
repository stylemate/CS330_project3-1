#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "threads/thread.h"
#include "filesys/file.h"

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

	/* FILE_SYSTEM */
	struct file *file;
	off_t ofs;
	size_t read_bytes;
	size_t zero_bytes;
	bool writable;
};

void sup_page_init (struct hash *);
void sup_page_destroy (struct hash *);

#endif
