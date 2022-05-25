#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"
#include "threads/thread.h"
#include "lib/kernel/hash.h"

struct page
{
  void *upage;
  block_sector_t sector;
  bool writable;
  struct hash_elem elem;
};

void page_table_create (struct hash *h);
void create_page_entry (struct hash *h, block_sector_t sector, void *upage, bool writable);
struct page *find_page_entry (struct hash *h, void *upage);
struct page *remove_page_entry (struct hash *h, void *upage);
void page_destroy (struct hash *h);

#endif