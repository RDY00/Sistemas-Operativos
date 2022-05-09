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
#include "devices/kbd.h"
#include "devices/acpi.h"
#include "devices/input.h"
#include "devices/serial.h"
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

/*Struct to implement swapping.*/
struct block* swap;

/*List of free slots in secondary storage.*/
static struct list free_slots;

/*Function to Initialize swapping proces.*/
/*It must be called in init.c.*/
void
swap_init()
{
  swap = block_get_role(BLOCK_SWAP);
  if(swap == NULL) {
    PANIC ("Error: Can't initialize swap block");
    NOT_REACHED ();
  }
}
