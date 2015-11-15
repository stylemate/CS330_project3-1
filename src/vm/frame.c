#include <hash.h>
#include <stdio.h>
#include "threads/synch.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "vm/frame.h"

struct hash frame_table;
struct lock frame_lock;

unsigned frame_hash (const struct hash_elem *f_, void *aux UNUSED);
bool frame_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);

void
frame_init (void)
{
	hash_init (&frame_table, frame_hash, frame_less, NULL);
	lock_init (&frame_lock);
}

/* Returns a hash value for frame f. */
unsigned
frame_hash (const struct hash_elem *f_, void *aux UNUSED)
{
  const struct frame *f = hash_entry (f_, struct frame, hash_elem);
  return hash_bytes (&f->addr, sizeof f->addr);
}

/* Returns true if frame a precedes frame b. */
bool
frame_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  const struct frame *a = hash_entry (a_, struct frame, hash_elem);
  const struct frame *b = hash_entry (b_, struct frame, hash_elem);

  return a->addr < b->addr;
}

void *
falloc_get_frame (void *upage, enum palloc_flags flags)
{
	if ((flags & PAL_USER) == 0)
		return NULL;	//check if the flag is of the user pool

	void *f = palloc_get_page (flags);

	if (f == NULL)
	{
		//evict
	}
	else
	{
		struct frame *frame = (struct frame*) malloc (sizeof (struct frame));
		frame->addr = f;
		frame->thread = thread_current ();
		frame->upage = upage;
		lock_acquire (&frame_lock);
		hash_insert (&frame_table, &frame->hash_elem);
		lock_release (&frame_lock);
	}
	return f;
}

void 
falloc_free_frame (void *frame)
{
	struct frame *f;
	struct hash_elem *e;
	struct frame *elem = (struct frame *) malloc (sizeof (struct frame));
	elem->addr = frame;

	e = hash_find (&frame_table, &elem->hash_elem);
	if (e == NULL)
		printf("No frame to free");
	else
	{
		f = hash_entry (e, struct frame, hash_elem);
		lock_acquire (&frame_lock);
		palloc_free_page (f->addr);
		hash_delete (&frame_table, &f->hash_elem);
		free (f);
		lock_release (&frame_lock);
	}
}
