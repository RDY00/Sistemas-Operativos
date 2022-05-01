#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  uint32_t *esp = f->esp;
  uint32_t syscall = *esp;
  esp++;
  
  switch(syscall) 
  {
    case SYS_WRITE: 
    {
      int fd = *esp;
      esp++;
      void* buffer = (void*)*esp;
      esp++;
      unsigned int size = *esp;
      
      putbuf(buffer, size);
      
      break;
    }
    case SYS_EXIT: 
    {
      int status = *esp;
      struct thread *t = thread_current ();
      t->process->exit_status = status;
      t->process->exited = true;

      if (t->is_waiting_child)
        sema_up (&t->parent->sema_exit);

      printf("%s: exit(%d)\n", t->name, status);
      thread_exit ();
      break;
    }
    case SYS_EXEC:
    {
      int tid = process_execute ((char *) *esp);
      struct thread *t = thread_current ();
      bool success = false;
      sema_down (&t->sema_load);

      if (tid != TID_ERROR)
      {
        struct process *pb = get_child_process (t, tid);
        success = pb->successful_load;
        if (!success)
        {
          list_remove (&pb->elem);
          pb->t->process = NULL;
          free (pb);
        }
      }

      f->eax = success ? tid : -1;
      break;
    }
    case SYS_WAIT:
    {
      f->eax = process_wait ((int) *esp);
      break;
    }
  }
}
