#include "vm/page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "lib/kernel/hash.h"
#include "lib/debug.h"

unsigned page_hash (const struct hash_elem *, void *);
bool page_less (const struct hash_elem *, const struct hash_elem *, void *);
void page_destructor (struct hash_elem *, void *);

/*Function to create a new page table. Which is a hashtable.*/
void
page_table_create (struct hash *h)
{
  hash_init (h, page_hash, page_less, NULL);
}

/*Function to create a new element for the page table. Which is a hash.*/
void
create_page_entry (struct hash *h, block_sector_t sector,
                   void *upage, bool writable)
{
  struct page *p = (struct page *) malloc (sizeof p);
  if (!p) PANIC ("Couldn't allocate page memory");
  p->upage = upage;
  p->sector = sector;
  p->writable = writable;
  hash_insert (h, &p->elem);
}

/*Function to void an element from the page table, the information of a page. Which is a hash.*/
struct page *
remove_page_entry (struct hash *h, void *upage)
{
  struct page p;
  p.upage = upage;
  struct hash_elem *e = hash_delete (h, &p.elem);
  return e != NULL ? hash_entry (e, struct page, elem) : NULL;
}

/*Function to find an elemente in the page table.*/
struct page *
find_page_entry (struct hash *h, void *upage)
{
  struct page p;
  p.upage = upage;
  struct hash_elem *e = hash_find (h, &p.elem);
  return e != NULL ? hash_entry (e, struct page, elem) : NULL;
}

/*Function to eliminate a page table entirely. */
void
page_destroy (struct hash *h)
{
  hash_destroy (h, page_destructor);
}

/*Function to otaint the position of a page in the page table.*/
unsigned
page_hash (const struct hash_elem *e, void *aux UNUSED)
{
  const struct page *p = hash_entry (e, struct page, elem);
  return hash_bytes (&p->upage, sizeof p->upage);
}

/*Funtion to determine the positonal relation of two different pages.*/
bool
page_less (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
  const struct page *pa = hash_entry (a, struct page, elem);
  const struct page *pb = hash_entry (b, struct page, elem);
  return pa->upage < pb->upage;
}

/*Function to empty the information of a page in the page table.*/
void
page_destructor (struct hash_elem *e, void *aux UNUSED)
{
  struct page *p = hash_entry (e, struct page, elem);
  free (p);
}
