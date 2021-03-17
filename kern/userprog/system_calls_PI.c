#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/include/kern/errno.h>

int sys_write(int fd, const void *buf, size_t nbytes, int* retval){

    // Check if file desrcriptor is valid // 
    if (fd == -1){
      *retval = -1;
      return EBADF;   
    }

    // Check if BUF address is valid // 

    if(buf == NULL ){
        *retval = -1;
        return EFAULT;
    }


}