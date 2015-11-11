#include <hash.h>
#include <stdio.h>
#include "threads/thread.h"
#include "threads/malloc.h"
#include "vm/page.h"

unsigned sup_page_hash (const struct hash_elem *p_, void *aux UNUSED);
bool sup_page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
void sup_page_action (struct hash_elem *e, void *aux UNUSED);

void
sup_page_init (struct hash *spt)
{
	hash_init (spt, sup_page_hash, sup_page_less, NULL);
}

/* Returns a hash value for supplementary page table p. */
unsigned
sup_page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  const struct sup_page *p = hash_entry (p_, struct sup_page, hash_elem);
  return hash_int ((int)p->addr);
}

/* Returns true if spt a precedes spt b. */
bool
sup_page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED)
{
  const struct sup_page *a = hash_entry (a_, struct sup_page, hash_elem);
  const struct sup_page *b = hash_entry (b_, struct sup_page, hash_elem);

  return a->addr < b->addr;
}

void
sup_page_action (struct hash_elem *e, void *aux UNUSED)
{
	struct sup_page *spt = hash_entry (e, struct sup_page, hash_elem);
	free (spt);
}

void
sup_page_destroy (struct hash *spt)
{
	hash_destroy (spt, sup_page_action);
}
