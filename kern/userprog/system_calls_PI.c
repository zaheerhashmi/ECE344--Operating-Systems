#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <thread.h>

int sys_write(int fd, const void *buf, size_t nbytes, int* retval){

    char* kernelBuffer;
    int copyError;
    
    // Check if file desrcriptor is valid // 
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO ){
      *retval = -1;
      return EBADF;   
    }

    // Check if BUF address is valid // 

    if(buf == NULL ){
        *retval = -1;
        return EFAULT;
    }

      kernelBuffer = kmalloc(nbytes+1);

      if(kernelBuffer == NULL){
        *retval = -1;
        return ENOSPC;
      }

      copyError = copyin(buf, kernelBuffer,nbytes);

      if(copyError != 0){
        *retval = -1;
        return copyError;
      }
      kernelBuffer[nbytes] = '\0';
      kprintf("%s",kernelBuffer);
      kfree(kernelBuffer);

      *retval = nbytes;
      return 0;

    // Simply print output to standard output using copyin and kprintf
    // Recall that buf is userlevel address we need to copy from user to a kernel buffer //

      


}

 int sys_read (int fd, void *buf, size_t buflen, int* retval){

    char charRead;
    int copyError;
   
    // Check if file desrcriptor is valid // 
    if (fd != STDIN_FILENO){
      *retval = -1;
      return EBADF;   
    }

    // Check if BUF address is valid // 

    if(buf == NULL ){
        *retval = -1;
        return EFAULT;
    }

    if(buflen != 1){
      *retval = -1;
      return EUNIMP;
    }


    charRead = getch();
    copyError = copyout(&charRead,buf,buflen);
    
    if(copyError != 0){
        *retval = -1;
        return copyError;
      }
  
    kprintf("%c",charRead);
    *retval = buflen;
    return 0;


} 

int sys_sleep(int seconds,int *retval){

  clocksleep(seconds);
  *retval = 0;
  return 0;

}


