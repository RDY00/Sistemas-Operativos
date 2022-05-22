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

void *insert_frame (struct page *);
void *get_kpage_swap (struct page *);

/*Struct to represent */
struct frame_entry
{
  void *kpage;
  struct thread *t;
  struct page *p;
  struct list_elem elem;
};

/* Function to initialize the Frame table. */
void
frame_init (void)
{
  list_init (&frames);
  lock_init (&vm_lock);
}

/*
Function to determine if a frame has been allocated correctly or
not. If it can't be allocated should return null.
*/
void *
palloc_swap (void *upage, bool writable)
{
  // lock_acquire (&vm_lock);
  struct page *p = create_page_entry (thread_current ()->pt, upage);
  p->writable = writable;
  void *kpage = get_kpage_swap (p);
  // lock_release (&vm_lock);
  return kpage;
}

void *
get_kpage_swap (struct page *p)
{
  void *kpage = insert_frame (p);

  if (kpage) return kpage;

  struct frame_entry *selected;
  selected = list_entry (list_pop_front (&frames), struct frame_entry, elem);

  selected->p->sector = swap_write (selected->kpage);
  selected->p->in_disk = true;
  pagedir_clear_page (selected->t->pagedir, selected->p->upage);
  free (selected);

  kpage = insert_frame (p); /* It should work this time */
  return kpage;
}

void *
insert_frame (struct page *p)
{
  void *kpage = palloc_get_page (PAL_USER);

  if (kpage)
  {
    struct frame_entry *fe = (struct frame_entry *) calloc (1, sizeof *fe);
    fe->t = thread_current ();
    fe->p = p;
    fe->kpage = kpage;
    list_push_back (&frames, &fe->elem);
    return kpage;
  }
  return NULL;
}

bool
activate_page (void *upage)
{
  // lock_acquire (&vm_lock);
  upage = pg_round_down (upage);
  struct thread *t = thread_current ();
  struct page *p = find_page_entry (t->pt, upage);
  ASSERT (!p);
  ASSERT (t->magic == 0xcd6abf4b);
  ASSERT (t->magic != 0xcd6abf4b);


  if (!p || !p->in_disk)
  {
    lock_release (&vm_lock);
    ASSERT (2==3);
    return false;
  }

  void *kpage = get_kpage_swap (p);
  swap_read (kpage, p->sector);
  p->in_disk = false;
  bool success = pagedir_set_page (t->pagedir, upage, kpage, true);
  lock_release (&vm_lock);
  ASSERT (4==3);
  return success;
}
