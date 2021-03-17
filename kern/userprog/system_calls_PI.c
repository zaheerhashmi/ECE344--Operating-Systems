#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <thread.h>
#include <clock.h>

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

int sys__time(time_t *seconds, int *nanoseconds, int *retval){

  int kernSeconds;
  int kernNanoseconds;
  int copyError;

 
  gettime((int *)&kernSeconds,&kernNanoseconds);

// if(seconds == NULL || nanoseconds == NULL ){
 //*retval = -1
 // return EFAULT;
//}
 
  // Ensure seconds and nano seconds are valid addresses // 

// Some times tester wants seconds sometimes nano seconds seperated the cases 
if(seconds != NULL){
  copyError = copyout(&kernSeconds,(void*)seconds,sizeof(int));
    
    if(copyError != 0){
        return copyError;
      }
  }

    if(nanoseconds != NULL){
    copyError = copyout(&kernNanoseconds,(void *)nanoseconds,sizeof(int));
    
    if(copyError != 0){
        return copyError;
      }
    }

  *retval = kernSeconds;
  return 0;
  
  

  // Needs to write the obtained values to seconds and nanoseconds // 

}

void sys_exit(int exitcode){
    kprintf("%d",exitcode);
    thread_exit();
}


