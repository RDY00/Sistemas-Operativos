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

      struct list *childs = &t->parent->child_processes;
      struct list_elem *e = list_begin (childs);
      struct process *pb;

      for (; e != list_end (childs); e = list_next (e))
      {
        pb = list_entry (e, struct process, elem);
        
        if (pb->tid == t->tid)
        {
          pb->exit_status = status;
          pb->exited = true;
          break;
        }
      }

      if (t->is_waiting_child)
        sema_up (&t->parent->sema_exit);

      printf("%s: exit(%d)\n", t->name, status);
      thread_exit ();
      break;
    }
    case SYS_EXEC:
    {
      int tid = process_execute ((char *) *esp);

      sema_down (&thread_current ()->sema_load);

      struct thread *t = thread_current ();
      struct list *l = &t->child_processes; 
      struct list_elem *e = list_begin (l);
      struct process *pb;
      bool success = false;

      for (; e != list_end (l); e = list_next (e))
      {
        pb = list_entry (e, struct process, elem);

        if (pb->tid == tid)
        {
          success = pb->successful_load;
          if (!success)
          {
            list_remove (e);
            free (pb);
          }
          break;
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
