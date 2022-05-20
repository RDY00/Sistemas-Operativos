#include "threads/init.h"
#include <console.h>
#include <debug.h>
#include <inttypes.h>
#include <limits.h>
#include <random.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
//#include "lib/kernel/hash.c"

#include <string.h>
#include "devices/shutdown.h"
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
#include "devices/block.h"
#include "lib/kernel/hash.h"

/*Struct to implement swapping.*/
struct block* swap;

/*List of free slots in secondary storage.*/
//List for FIFO. Otherwise hash.
static struct list free_slots;

/* Represents how many sectors are needed to store a page */
static size_t SECTORS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;
static size_t swap_size_in_page (void);


/*Struct to compare pages, for recovering a page from sencondary storage. This struct could go in thread.h for lettíng everý thread know what pages belong to it.*/
struct page_swap
{
  void *upage;              /*Page to bring from H*/
  block_sector_t swap_slot; /* What sector or block the page is (in secondary storage).*/

};

/*Function to Initialize swapping proces.*/
/*It must be called in init.c.*/
void
init_swap()
{
  swap = block_get_role (BLOCK_SWAP);//512 bytes repartidos en 4k.Es decir debemos saber cuántos de esos blocks para una pagina.
  if(swap == NULL)
  {
    PANIC ("Error: Can't initialize swap block");
    NOT_REACHED ();
  }

  list_init(&free_slots);
}

/*Funciton to look for pages in */
bool
swap_find(void *fault_addr)
{
  //pagedir_set_page(thread_current()->pagedir, fault_addr->upage);
}

/*Function to retrive information from secondary storage.*/
void
swap_in(int page, void *uva) {

  int counter = 0;

  while(counter < SECTORS_PER_PAGE){

    block_read (swap, page * SECTORS_PER_PAGE + counter, uva + counter * BLOCK_SECTOR_SIZE);

    counter++;
  }

  /*Here we should use function to free a slot.*/
}

/* Find an available swap slot and dump in the given page represented by UVA
   If failed, return SWAP_ERROR
   Otherwise, return the swap slot index */
void
swap_out(const void *uva)
{
  /*Aquí no sé si poner size_t en vez de int o qué poner jaja*/
  int page = list_pop_back(&free_slots);

  if(page == NULL)
    PANIC("Cant swap_out.");

  int counter = 0;

  while(counter < SECTORS_PER_PAGE){

    block_write (swap, page * SECTORS_PER_PAGE + counter, uva + counter * BLOCK_SECTOR_SIZE);

    counter++;
  }

  return page;
}

/* Returns how many pages the swap device can contain, which is rounded down */
static size_t
swap_size_in_page ()
{
  return block_size (swap) / SECTORS_PER_PAGE;
}
