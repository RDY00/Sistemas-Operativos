#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "devices/block.h"
#include "lib/kernel/hash.h"

struct page_table
{
  struct hash pt_hash;
};

struct page
{
  void *upage;
  block_sector_t sector;
  bool in_disk;
  bool writable;
  struct hash_elem elem;
};

struct page_table *page_table_create (void);
struct page *create_page_entry (struct page_table *pt, void *upage);
struct page *find_page_entry (struct page_table *pt, void *upage);
void remove_page_entry (struct page_table *pt, void *upage);
void page_destroy (struct page_table *pt);

#endif