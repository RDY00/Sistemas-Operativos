#include "vm/swap.h"
#include "devices/block.h"
#include "lib/kernel/list.h"
#include "lib/kernel/bitmap.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "vm/frame.h"

/* Struct to implement swapping. */
struct block *swap_block;

static struct bitmap *sectors;
/* List of used slots in secondary storage. */
static struct list used_sectors;

/* Represents how many sectors are needed to store a page */
static size_t SECTORS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;
static size_t swap_size_in_page (void);

block_sector_t find_free_sectors (void);

/* Struct to compare pages, for recovering a page from sencondary storage.
   This struct could go in thread.h for letting every thread know what
   pages belong to it. */
struct page_swap
{
  void *upage;              /* Page to bring from H */
  block_sector_t swap_slot; /* What sector or block the page is (in secondary storage). */
  struct list_elem elem;
};

/* Function to initialize swapping process. */
/* It must be called in init.c. */
void
init_swap (void)
{
  swap_block = block_get_role (BLOCK_SWAP);

  if (swap_block == NULL)
    PANIC ("Error: Can't initialize swap block");

  sectors = bitmap_create (block_size (swap_block) / SECTORS_PER_PAGE);
  list_init (&used_sectors);
}

/* Function to look for pages in */
block_sector_t
swap_find (void *upage)
{
  struct list_elem *e = list_begin (&used_sectors);
  struct page_swap *ps;

  for (; e != list_end (&used_sectors); e = list_next (e))
  {
    ps = list_entry (e, struct page_swap, elem);
    if (ps->upage = upage) {
      list_remove (e);
      swap_read ()
      return true;
    }
  }

  return false
  //pagedir_set_page(thread_current()->pagedir, fault_addr->upage);
}

/* Function to retrive information from secondary storage. */
bool
swap_read (void *upage)
{
  struct list_elem *e = list_begin (&used_sectors);
  struct page_swap *ps;
  uint8_t *temp = upage;
  block_sector_t s;
  bool found = false;

  for (; e != list_end (&used_sectors); e = list_next (e))
  {
    ps = list_entry (e, struct page_swap, elem);
    if (ps->upage = upage) {
      found = true;
      s = ps->swap_slot;
      bitmap_flip (sectors, s);
      list_remove (e);
      break;
    }
  }

  if (!found) return false;

  uint8_t *kpage = palloc_swap (upage);
  pagedir_set_page (thread_current ()->pagedir, upage, kpage);

  for (; s < SECTORS_PER_PAGE; s++, temp += BLOCK_SECTOR_SIZE)
    block_read (swap_block, s, temp);

  return true;
}

/* Find an available swap slot and dump in the given page represented by UVA
   If failed, return "can't swap."
   Otherwise, return the swap slot index */
void
swap_write (void *upage, void *kpage)
{
  block_sector_t s = find_free_sectors ();
  uint8_t *temp = kpage;

  struct page_swap ps;
  ps.upage = upage;
  ps.swap_slot = s;
  list_push_back (&used_sectors, &ps.elem);

  for (; s < SECTORS_PER_PAGE; s++, temp += BLOCK_SECTOR_SIZE)
    block_write (swap_block, s, temp);

  palloc_free_page (kpage);
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

/* Returns how many pages the swap device can contain, which is rounded down */
static size_t
swap_size_in_page (void)
{
  return block_size (swap_block) / SECTORS_PER_PAGE;
}
