#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "lib/kernel/list.h"

struct process 
{
  tid_t tid;
  struct thread *t;
  int exit_status;
  struct list_elem elem;
  bool successful_load;
  bool exited;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

struct process * get_child_process (struct thread *, tid_t);

#endif /* userprog/process.h */
