#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "lib/kernel/list.h"
#include "vm/swap.h"
#include "vm/page.h"

/*
Usar un lock para concurrencias.
*/

//Si en page-parallel ocurre algo raro puede ser el caso especifico donde wait está mal implementada.
/*Global list for the Frames Table.*/
static struct list frames;
/*Lock to handle sincronization for proces on frame table.*/
static struct lock vm_lock;
static struct lock activate_lock;


void *swap_frames (void);
struct frame_entry *select_swap_frame (void);
void create_frame (void *upage, void *kpage, bool writable);

/*Struct to represent a memory frame.*/
struct frame_entry
{
  struct thread *t;
  void *upage;
  void *kpage;
  bool writable;
  struct list_elem elem;
};

/* Function to initialize the Frame table and the lock. */
void
frame_init (void)
{
  list_init (&frames);
  lock_init (&vm_lock);
  lock_init (&activate_lock);
}

/*
Function to determine if a frame has been allocated correctly or
not. If it can't be allocated returns null.
*/
void *
palloc_swap (void *upage, bool writable)
{
  lock_acquire (&vm_lock);
  // printf("Im in with upage=%p\n", upage);
  void *kpage = palloc_get_page (PAL_USER);
  if (!kpage) kpage = swap_frames ();
  create_frame (upage, kpage, writable);
  lock_release (&vm_lock);
  // printf ("end with page =%p\n", upage);
  return kpage;
}

void *
swap_frames (void)
{
  struct frame_entry *selected = select_swap_frame ();
  block_sector_t s = swap_write (selected->kpage);
  create_page_entry (&selected->t->pt, s, selected->upage, selected->writable);
  // printf ("last action with page =%p\n", selected->upage);
  pagedir_clear_page (selected->t->pagedir, selected->upage);
  void *kpage = selected->kpage;
  free (selected);
  return kpage;
}

struct frame_entry *
select_swap_frame (void)
{
  return list_entry (list_pop_front (&frames), struct frame_entry, elem);
}

void
create_frame (void *upage, void *kpage, bool writable)
{
  struct frame_entry *fe = (struct frame_entry *) malloc (sizeof *fe);
  fe->t = thread_current ();
  fe->kpage = kpage;
  fe->upage = upage;
  fe->writable = writable;
  list_push_back (&frames, &fe->elem);
}

/*Function to load the information of a page recently recovered from disk.*/
bool
activate_page (struct thread *t, void *upage)
{
  // if (activate_lock.holder == t) return true;
  // lock_acquire (&activate_lock);
  upage = pg_round_down (upage);

  // struct thread *t = thread_current ();
  struct page *p = find_page_entry (&t->pt, upage);
  // printf("Activating page=%p, %p\n", upage, p);

  if (p) {
    if (p->loaded) return true;
    else p->loaded = true;
  } else return false;

  // if (!p || !p->loaded) {
  //   printf("Shouldn't be here, page=%p, %d\n", upage, p->loaded);
  //   return false;
  // }

  // if (p->loaded) return true;

  // p->loaded = true;
  // printf("Reached with page %p\n", upage);
  // printf("Activating page=%p\n", upage);
  void *kpage = palloc_swap (upage, p->writable);
  // printf("Success in activate_page, page=%p\n", upage);
  swap_read (kpage, p->sector);
  bool success = pagedir_set_page (t->pagedir, upage, kpage, p->writable);
  remove_page_entry (&t->pt, upage);
  free (p);
  lock_release (&activate_lock);
  return success;
}
