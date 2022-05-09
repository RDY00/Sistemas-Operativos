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


/*Global list for the Frames Table.*/
static struct list tbf;


/*Function to initialize the Frame table.*/
void
tbf_init(void)
{

}

/*
Function to determine if a frame has been allocated correctly or
not.
*/
void *
palloc2(void)
{
  /* code */
}
