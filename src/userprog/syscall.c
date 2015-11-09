#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "devices/input.h"
#include "devices/shutdown.h"

#define USR_BOT ((void *) 0x08048000)
static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
    lock_init(&file_lock);
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* According to first element pointed by esp, invoke systemcalls.
   Values taken as syscall# are defined in vaddr.h.
   This pointer, esp, should be valid. Since some systemcalls need
   arguments, we need to verify everytime we retrieve data from the
   stack. is_val_ptr and ret_args functions will do this job */
static void
syscall_handler (struct intr_frame *f UNUSED)
{
    int arg[3];
    int esp;
    is_val_ptr((const void *)f->esp);
    esp = (int)utk_ptr((const void *)f->esp);
    switch(* (int *)esp){
        /* Halt the operating system.
           No arg is needed */
        case SYS_HALT:
            halt();
            break;

        /* Terminate this process.
           Takes 1 arg */
        case SYS_EXIT:
            ret_args(f->esp, &arg[0], 1);
            exit(arg[0]);
            break;

        /* Start another process.
           Takes 1 arg */
        case SYS_EXEC:
            ret_args(f->esp, &arg[0], 1);
            f->eax = exec((const char*)arg[0]);
            break;

        /* Wait for a child process to die.
           Takes 1 arg */
        case SYS_WAIT:
            ret_args(f->esp, &arg[0], 1);
            f->eax = wait((int)arg[0]);
            break;

        /* Create a file.
           Takes 2 args */
        case SYS_CREATE:
            ret_args(f->esp, &arg[0], 2);
            f->eax = create((const char *)arg[0], (unsigned)arg[1]);
            break;

        /* Delete a file.
           Takes 1 arg */
        case SYS_REMOVE:
            ret_args(f->esp, &arg[0], 1);
            f->eax = remove((const char *)arg[0]);
            break;

        /* Open a file.
           Takes 1 arg */
        case SYS_OPEN:
            ret_args(f->esp, &arg[0], 1);
            f->eax = open((const char *)arg[0]);
            break;

        /* Obtain a file's size.
           Takes 1 arg */
        case SYS_FILESIZE:
            ret_args(f->esp, &arg[0], 1);
            f->eax = filesize(arg[0]);
            break;

        /* Read from a file.
           Takes 3 args */
        case SYS_READ:
            ret_args(f->esp, &arg[0], 3);
            f->eax = read(arg[0], (void *)arg[1], (unsigned)arg[2]);
            break;

        /* Write to a file.
           Takes 3 args */
        case SYS_WRITE:
            ret_args(f->esp, &arg[0], 3);
            f->eax = write(arg[0], (const void *)arg[1], (unsigned)arg[2]);
            break;

        /* Change position in a file.
           Takes 2 args */
        case SYS_SEEK:
            ret_args(f->esp, &arg[0], 2);
            seek(arg[0], (unsigned)arg[1]);
            break;

        /* Report current position in a file.
           Takes 1 arg */
        case SYS_TELL:
            ret_args(f->esp, &arg[0], 1);
            f->eax = tell(arg[0]);
            break;

        /* Close a file.
           Takes 1 arg */
        case SYS_CLOSE:
            ret_args(f->esp, &arg[0], 1);
            close(arg[0]);
            break;
    }
}

/* Terminates Pintos by calling shutdown_power_off()
   (declared in ‘devices/shutdown.h’). */
void halt(void){
    shutdown_power_off();
}

/* Terminates the current user program, returning status to the kernel.
   If the process’s parent waits for it, this is the status that
   will be returned. Conventionally, a status of 0 indicates success
   and nonzero values indicate errors. */
void exit(int status){
    struct thread *cur = thread_current();
    /* To return its terminating status to parent. */
    if(is_running(cur->parent)) cur->child->status = status;

    printf ("%s: exit(%d)\n", cur->name, cur->child->status);
    thread_exit();
}

/* Runs the executable whose name is given in cmd line, passing any
   given arguments, and returns the new process’s program id (pid).
   Must return pid -1, which otherwise should not be a valid pid, if the program cannot load or run for any reason. Thus, the parent
   process cannot return from the exec until it knows whether the
   child process successfully loaded its executable.
   You must use appropriate synchronization to ensure this. */
int exec(const char *cmd_line){
    char* fn = (char *)utk_ptr((const void *)cmd_line);
    int pid = process_execute((const char *)fn);
    struct child_process* child = search_child_process(pid);
    if(child == NULL) return -1;

    /* Investigate child's load status */
    while(child->load_status == 0) sema_down(&child->load_sema);
    if(child->load_status == -1 ){
        child_process_remove(child);
        return -1;
    }
    return pid;
}

/* Waits for a child process pid and retrieves the child’s exit status. */
int wait(int pid){
    return process_wait(pid);
}

/* Creates a new file called file initially initial size bytes in
   size. Returns true if successful, false otherwise. */
bool create(const char *file, unsigned initial_size){
    is_val_str((const void *)file);
    char* fn = (char *)utk_ptr((const void *)file);
    lock_acquire(&file_lock);
    bool success = filesys_create((const char *)fn, initial_size);
    lock_release(&file_lock);
    return success;
}

/* Deletes the file called file. Returns true if successful,
   false otherwise. */
bool remove(const char *file){
    is_val_str((const void *)file);
    char* fn = (char *)utk_ptr((const void *)file);
    lock_acquire(&file_lock);
    bool success = filesys_remove((const char *)fn);
    lock_release(&file_lock);
    return success;
}

/* Opens the file called file. Returns a nonnegative integer handle
   called a “file descriptor”(fd), or -1 if the file could not be
   opened. */
int open(const char *file){
    is_val_str((const void *)file);
    char* fn = (char *)utk_ptr((const void *)file);
    lock_acquire(&file_lock);
    struct file *f = filesys_open((const char *)fn);
    if(f == NULL){
        lock_release(&file_lock);
        return -1;
    }

    /* filesys_open returns file returned by file_open.
       It returns NULL if there is no such a file. */
    struct filestruct *fst = malloc(sizeof(struct filestruct));
    if(fst == NULL) return -1;
    fst->file = f;
    fst->fd = thread_current()->fd;
    thread_current()->fd++;
    list_push_back(&thread_current()->files, &fst->elem);
    lock_release(&file_lock);
    return fst->fd;
}

/* Returns the size, in bytes, of the file open as fd.
   Returns -1 if it doesn't exist. */
int filesize(int fd){
    struct file *f = search_file(fd);
    lock_acquire(&file_lock);
    if(f == NULL){
        lock_release(&file_lock);
        return -1;
    }
    int length = (int)file_length(f);
    lock_release(&file_lock);
    return length;
}

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read (0 at end of file),
   or -1 if the file could not be read. */
int read(int fd, void *buffer, unsigned size){
    is_val_buff((const void *)buffer, size);
    void *kbuf = utk_ptr((const void *)buffer);
    if(fd == 0){
        /* STDIN */
        uint8_t *local_buff = (uint8_t *)kbuf;
        unsigned i;
        for(i = 0; i< size ; i++){
            local_buff[i] = input_getc();
        }
        return size;
    }
    lock_acquire(&file_lock);
    struct file *f = search_file(fd);
    if(f == NULL){
        lock_release(&file_lock);
        return -1;
    }
    int nob = file_read(f, kbuf, size);
    lock_release(&file_lock);
    return nob;
}

/* Writes size bytes from buffer to the open file fd.
   Returns the number of bytes actually written, which may be less
   than size if some bytes could not be written. */
int write(int fd, const void *buffer, unsigned size){
    is_val_buff(buffer, size);
    void *kbuf = utk_ptr(buffer);
    if(fd == 1){
        /* STDOUT */
        putbuf((const void *)kbuf, size);
        return size;
    }
    lock_acquire(&file_lock);
    struct file *f = search_file(fd);
    if(f == NULL){
        lock_release(&file_lock);
        return -1;
    }
    int nob = file_write(f, kbuf, size);
    lock_release(&file_lock);
    return nob;
}

/* Changes the next byte to be read or written in open file fd
   to position, expressed in bytes from the beginning of the file. */
void seek (int fd, unsigned position){
    struct file *f = search_file(fd);
    lock_acquire(&file_lock);
    if(f == NULL){
        lock_release(&file_lock);
        return;
    }
    file_seek(f, position);
    lock_release(&file_lock);
}

/* Returns the position of the next byte to be read or written in
   open file fd, expressed in bytes from the beginning of the file. */
unsigned tell(int fd){
    struct file *f = search_file(fd);
    lock_acquire(&file_lock);
    if(f == NULL){
        lock_release(&file_lock);
        return -1;
    }
    unsigned pos = file_tell(f);
    lock_release(&file_lock);
    return pos;
}

/* Closes file descriptor fd. Exiting or terminating a process
   implicitly closes all its open file descriptors, as if by calling
   this function for each one. */
void close(int fd){
    struct thread *t = thread_current();
    struct list_elem *e = list_begin(&t->files);
    while(e != list_end(&t->files)){
        struct filestruct *fst = list_entry(e, struct filestruct, elem);
        if(fst->fd == fd){
            file_close(fst->file);
            list_remove(&fst->elem);
            free(fst);
            return;
        }
        e = list_next(e);
    }
}

/* Checks if the given pointer PTR is valid(> USR_BOT, < PHYS_BASE)
   or not. Using is_user_vaddr for checking it. */
void is_val_ptr(const void *ptr){
    if(!is_user_vaddr(ptr) || ptr < USR_BOT)
        exit(-1);
    if(!pagedir_get_page (thread_current ()->pagedir, ptr))
        exit(-1);
    if(ptr == NULL)
        exit(-1);
}

/* Checks if the given buffer pointer BUFF is valid with the size N */
void is_val_buff(const void *buffer, unsigned size){
    char *ptr = (char *)buffer;
    unsigned i;
    for(i = 0; i < size; i++){
        is_val_ptr((const void *)ptr);
        ptr++;
    }
}

/* Checks if the given string pointer is valid by calling
   is_val_ptr per every character's address. */
void is_val_str (const void *str){
    while (* (char *) utk_ptr(str) != 0){
        str = (char *) str + 1;
    }
}

/* Retrieves arguments from the given pointer PTR.
   Valitity checking is required for every N args.
   The list ARG will have arguments after. */
void ret_args(void *ptr, int *arg, int n){
    int i;
    int *local_ptr;
    local_ptr = (int *)ptr;
    for(i = 0; i < n; i++){
        local_ptr = local_ptr + 1;
        is_val_ptr((const void *) local_ptr);
        arg[i] = *local_ptr;
    }
}

/* Gets child process' PID as an argument, initialize its
   child_process structure. Returns child_process */
struct child_process * child_proc_init (tid_t pid){
    struct child_process *child = malloc(sizeof(struct child_process));
    child->pid = pid;
    child->load_status = 0;
    child->called = false;
    child->killed = false;
    sema_init(&child->killed_sema, 0);
    sema_init(&child->load_sema, 0);
    list_push_back(&thread_current()->children, &child->elem);
    return child;
}

/* process.c will call this function, in order to finish releasing
   resources of process. Free child_processes*/
void child_process_destroy (void){
    struct thread *cur = thread_current();
    struct list_elem *e = list_begin(&cur->children);
    while(e != list_end(&cur->children)){
        struct child_process *child = list_entry(e, struct child_process, elem);
        e = list_remove(&child->elem);
        free(child);
    }
}

/* Gets rid of the given child_process from the list
   children. */
void child_process_remove(struct child_process* child){
    list_remove(&child->elem);
    free(child);
}

/* Gets userprocess pointer, and returns kernel pointer. */
void * utk_ptr(const void *vptr){
    is_val_ptr((const void *)vptr);
    void *kptr = pagedir_get_page(thread_current()->pagedir, vptr);
    if(kptr == NULL) exit(-1);
    return kptr;
}

/* Returns a child_process ptr with the given PID.
   If there isn't a child_process with the given PID,
   returns NULL */
struct child_process * search_child_process(int pid){
    struct thread *cur = thread_current();
    struct list_elem *e = list_begin(&cur->children);
    while(e != list_end(&cur->children)){
        struct child_process *child = list_entry(e, struct child_process, elem);
        if(child->pid == pid) return child;
        e = list_next(e);
    }
    return NULL;
}

/* Returns a file with the given FD.
   If there isn't a file with the given FD, returns NULL */
struct file * search_file(int fd){
    struct thread *t = thread_current();
    struct list_elem *e = list_begin(&t->files);
    while(e != list_end(&t->files)){
        struct filestruct *fst = list_entry(e, struct filestruct, elem);
        if(fst->fd == fd) return fst->file;
        e = list_next(e);
    }
    return NULL;
}
