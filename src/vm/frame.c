#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "lib/kernel/list.h"
#include "vm/swap.h"

/*
Usar un lock para concurrencias.
*/

//Si en page-parallel ocurre algo raro puede ser el caso especifico donde wait está mal implementada.
/*Global list for the Frames Table.*/
static struct list frames;
/*Lock to handle sincronization for proces on frame table.*/
static struct lock vm_lock;

/*Struct to represent */
struct frame_entry
{
  struct thread *t;
  void *upage;
  void *kpage;
  struct list_elem elem;
};

/* Function to initialize the Frame table. */
void
frames_init (void)
{
  list_init (&frames);
  lock_init (&vm_lock);
}

/*
Function to determine if a frame has been allocated correctly or
not. If it can't be allocated should return null.
*/
void *
palloc_swap (void *upage)
{
  void *kpage = palloc_get_page (PAL_USER);

  if (kpage)
  {
    struct frame_entry fe;
    fe.t = thread_current ();
    fe.upage = upage;
    fe.kpage = kpage;
    list_push_back (&frames, &fe.elem);
    return kpage;
  }

  struct frame_entry *selected;
  selected = list_entry (list_pop_front (&frames), struct frame_entry, elem);

  //FIFO only use pop. Otherwise it may require an iteration to find the suitable page to eliminate.

  pagedir_clear_page (selected->t->pagedir, selected->upage);
  swap_
  
}

/*
Function to free a slot.Tal vez esta función está de más ya que el sacar de esa lista lo manejamos en palloc_swap.
*/
void *
release_slot(void)
{
  list_pop_front(&frames);
}
