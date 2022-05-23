#include "vm/page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "lib/kernel/hash.h"
#include "lib/debug.h"

unsigned page_hash (const struct hash_elem *, void *);
bool page_less (const struct hash_elem *, const struct hash_elem *, void *);
void page_destructor (struct hash_elem *, void *);

/*Function to create a new page table. Which is a hashtable.*/
struct page_table *
page_table_create (void)
{
  struct page_table *pt = (struct page_table *) calloc (1, sizeof *pt);
  hash_init (&pt->pt_hash, page_hash, page_less, NULL);
  return pt;
}

/*Function to create a new element for the page table. Which is a hash.*/
struct page *
create_page_entry (struct page_table *pt, void *upage)
{
  struct page *p = (struct page *) calloc (1, sizeof p);
  p->upage = upage;
  hash_insert (&pt->pt_hash, &p->elem);
  return p;
}

/*Function to void an element from the page table, the information of a page. Which is a hash.*/
void
remove_page_entry (struct page_table *pt, void *upage)
{
  struct page p;
  p.upage = upage;
  struct hash_elem *e = hash_delete (&pt->pt_hash, &p.elem);
  free (hash_entry (e, struct page, elem));
}

/*Function to find an elemente in the page table.*/
struct page *
find_page_entry (struct page_table *pt, void *upage)
{
  struct page p;
  p.upage = upage;
  struct hash_elem *e = hash_find (&pt->pt_hash, &p.elem);
  return e != NULL ? hash_entry (e, struct page, elem) : NULL;
}

/*Function to eliminate a page table entirely. */
void
page_destroy (struct page_table *pt)
{
  hash_destroy (&pt->pt_hash, page_destructor);
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
