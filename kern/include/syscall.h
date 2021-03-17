#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_write(int fd, const void *buf, size_t nbytes, int* retval);
int sys_read (int fd, void *buf, size_t buflen, int* retval);


#endif /* _SYSCALL_H_ */
