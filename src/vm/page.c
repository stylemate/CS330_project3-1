#include "threads/thread.h"
#include "vm/page.h"

struct hash sup_page_table;

unsigned sup_page_hash (const struct hash_elem *p_, void *aux UNUSED);
bool sup_page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);

void
sup_page_init (void)
{
	hash_init (&sup_page_table, sup_page_hash, sup_page_less, NULL);
}

/* Returns a hash value for supplementary page table p. */
unsigned
sup_page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
  const struct sup_page *p = hash_entry (p_, struct sup_page, hash_elem);
  return hash_bytes (&p->addr, sizeof p->addr);
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

