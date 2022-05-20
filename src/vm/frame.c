#include "threads/init.h"
#include <console.h>
#include <debug.h>
#include <inttypes.h>
#include <limits.h>
#include <random.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "devices/timer.h"
#include "devices/vga.h"
#include "devices/rtc.h"
#include "threads/interrupt.h"
#include "threads/io.h"
#include "threads/loader.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/thread.h"

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
  uint8_t *upage;
  uint8_t *kpage;
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
palloc_swap (uint8_t *upage)
{
  uint8_t *kpage = palloc_get_page (PAL_USER);

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
  selected = list_entry (list_pop_front (&frame), struct frame_entry, elem);

  //FIFO only use pop. Otherwise it may require an iteration to find the suitable page to eliminate.

  pagedir_clear_page (selected->t->pagedir, upage);
}

/*
Function to free a slot.
*/
void *
release_slot(void) 
{

}
