#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>
#include <kern/unistd.h>


/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

int sys_write(int fd, const void *buf, size_t nbytes, int* retval);

int sys_read (int fd, void *buf, size_t buflen, int* retval);

int sys_sleep(int seconds, int* retval);

int sys__time(time_t *seconds, int *nanoseconds, int *retval);


#endif /* _SYSCALL_H_ */
