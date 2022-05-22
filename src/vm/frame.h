#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>

void frame_init (void);
void *palloc_swap (void *, bool);
bool activate_page (void *upage);


#endif
