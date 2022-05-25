#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/thread.h"

void frame_init (void);
void *palloc_swap (void *, bool);
bool activate_page (struct thread *t, void *upage);

#endif
