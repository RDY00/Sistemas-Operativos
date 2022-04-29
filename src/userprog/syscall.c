#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/thread.c"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
  uint32_t* esp = f->esp;
  uint32_t syscall = *esp;
  esp++;

  switch(syscall)
  {
    case SYS_WRITE:
    {
      int fd = *esp;
      esp++;
      void* buffer = (void *) *esp;
      esp++;
      unsigned int size = *esp;

      putbuf(buffer, size);

      break;
    }
    case SYS_EXEC:
    {
      char* cmd = (char *) *esp;
      sema_down (&thread_current ()->wait);
      f->eax = process_execute (cmd);
      break;
    }
    case SYS_WAIT:
    {
      int pid = (int) *esp;
      f->eax = process_wait (pid);
      break;
    }
    case SYS_EXIT:
    {
      int status = *esp;
      struct thread* current = thread_current ();
      printf("%s: exit(%d)\n", current->name, status);

      if (current->parent != NULL)
      {
        current->pb->exit_status = status;
        sema_up (&current->parent->wait);
      }

      thread_exit ();
      break;
    }
  }
}
