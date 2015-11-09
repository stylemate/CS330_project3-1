#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <list.h>
#include "threads/thread.h"


struct lock file_lock;

struct filestruct{
    int fd;
    struct file *file;
    struct list_elem elem;
};


void syscall_init (void);
void halt(void);
void exit(int);
int exec(const char *);
int wait(int);
bool create(const char *, unsigned );
bool remove(const char *);
int open(const char *);
int filesize(int);
int read(int, void *, unsigned );
int write(int , const void *, unsigned );
void seek (int , unsigned );
unsigned tell(int );
void close(int );
void is_val_ptr(const void *);
void is_val_buff(const void *, unsigned);
void is_val_str (const void *);
void ret_args(void *, int *, int);
struct child_process* child_proc_init (tid_t);
void child_process_destroy(void);
void child_process_remove(struct child_process*);
void * utk_ptr(const void *);
struct child_process * search_child_process(int);
struct file * search_file(int);


#endif /* userprog/syscall.h */
