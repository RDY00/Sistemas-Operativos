#ifndef VM_SWAP_H
#define VM_SWAP_H

void swap_read(int page, void *uva);
void swap_write(const void *uva);

#endif
