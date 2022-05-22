#include "vm/swap.h"
#include <stdbool.h>
#include "devices/block.h"
#include "lib/kernel/list.h"
#include "lib/kernel/bitmap.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "vm/frame.h"
#include "vm/page.h"

/* Struct to implement swapping. */
struct block *swap_block;
static struct bitmap *sectors;

/* Represents how many sectors are needed to store a page */
static size_t SECTORS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

block_sector_t find_free_sectors (void);

/* Function to initialize swapping process. */
/* It must be called in init.c. */
void
init_swap (void)
{
  swap_block = block_get_role (BLOCK_SWAP);

  if (swap_block == NULL)
    PANIC ("Error: Can't initialize swap block");

  sectors = bitmap_create (block_size (swap_block) / SECTORS_PER_PAGE);
}

/* Function to look for pages in */
block_sector_t
swap_find (void *upage)
{
  struct thread *t = thread_current ();
  struct page *p = find_page_entry (t->pt->pt_hash, upage);

  //pagedir_set_page(thread_current()->pagedir, fault_addr->upage);
}

/* Function to retrive information from secondary storage. */
void
swap_read (void *kpage, block_sector_t s)
{
  uint8_t *temp = kpage;

  for (; s < SECTORS_PER_PAGE; s++, temp += BLOCK_SECTOR_SIZE)
    block_read (swap_block, s, temp);
}

/* Find an available swap slot and dump in the given page represented by UVA
   If failed, return "can't swap."
   Otherwise, return the swap slot index */
block_sector_t
swap_write (void *upage, void *kpage)
{
  block_sector_t s = find_free_sectors ();
  uint8_t *temp = kpage;

  for (; s < SECTORS_PER_PAGE; s++, temp += BLOCK_SECTOR_SIZE)
    block_write (swap_block, s, temp);

  return s;
}

block_sector_t
find_free_sectors (void)
{
  block_sector_t s = bitmap_scan_and_flip (swap_block, 0, 1, false);

  if (s == BITMAP_ERROR)
    PANIC("Couldn't find free sectors to allocate page");
  else
    return s * SECTORS_PER_PAGE;
}