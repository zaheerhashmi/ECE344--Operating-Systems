#include <types.h>
#include <lib.h>
#include <syscall.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <thread.h>
#include <clock.h>
#include <curthread.h>
#include <addrspace.h>
#include <machine/spl.h>
#include <synch.h>
#include <scheduler.h>
#include <pid_system.h>


extern struct lock* forkLock;
extern struct pid *pidHead;

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


pid_t sys_getpid(int *retval){
  *retval =  curthread->pidValue; 
  return 0;
  //kprintf("Get pid of the current process");
  //return 0;
}

int sys_fork (struct trapframe *tf, int *retval){  
  
  int s;
	s = splhigh();


  
  // Set up required data structures for the child // 

  int errorCode;
  pid_t parentPID; // passed as argument to stub functions: Debugging purposes // 
  parentPID = curthread->pidValue;
  struct trapframe * parentTrapframe = tf;
 
  struct thread* childThread;
  struct addrspace* childAddrspace;
  struct trapframe * childTrapframe = (struct trapframe *) kmalloc(sizeof(struct trapframe));

    if(childTrapframe == NULL){
        kprintf("Out of memory \n");
        *retval = -1;
        splx(s);
        return ENOMEM;
    }
    
  *(childTrapframe) = *(parentTrapframe); // Parent trap frame to child //
      

   
   // Child inherits parents address space; Using functions from dumbvm for now //
    errorCode=  as_copy(curthread->t_vmspace,&(childAddrspace));  

    if(errorCode){
        kfree(childTrapframe);
        *retval = -1;
        splx(s);
        return errorCode;
    }

  errorCode = thread_fork(curthread->t_name, 
  childTrapframe, (unsigned long)NULL,
  (void *)childProcstub, &childThread);

    if(errorCode){
        kfree(childTrapframe);
        as_destroy(childAddrspace);
        *retval = -1;
        splx(s);
        return errorCode;
    }

  childThread->parentPid = parentPID;
  childThread->t_vmspace = childAddrspace;
  as_activate(curthread->t_vmspace);

    // Returning child pid to parent // 
    *retval =  childThread->pidValue; 
      // print_run_queue(); use for debugging
    splx(s);
    return 0;

} 

int sys_waitpid(pid_t pid, int *status, int options, int *retval){
  
  // Error Handling Code //  
    
      // Making sure that options is zero we dont support any options//
        if(options != 0){
          *retval = -1;
          return EINVAL;
        }
      
      // Making sure status point is not null // 
        if(status == NULL){
          *retval = -1;
          return EFAULT;
        }
      
      // Only allow parent to do do waitpid for simplicity; shared data structure; if NULL then it means search failed //
        int s = splhigh(); 
        struct pid* checkChild = pid_search(pidHead,pid);
        splx(s);
        // This means we couldnt find process: waitpid has failed //
        if(checkChild == NULL){
          *retval = -1;
          return -1;
        }
        /* else we found a process with given pId; ensure it is 
        1) a child and 
        2) has it exited -> if it has exited return exit code;
                         -> if it hasnt exited we will not wait for it to exit  */
        if(curthread->pidValue == checkChild->pPid){
          if(checkChild->didExit == 1){
            *status = 1;
            *retval = checkChild->pidValue;
            return 0;
          }
          else{
            //if the child did not exit, it will be holding this semaphore
            //and we will be waiting here
            P(checkChild->parentSem);
            V(checkChild->parentSem);
            *status = 1;
            *retval = checkChild->pidValue;
            return 0;
            kprintf("I am gonna wait for child \n");
          }
        }

        else{
          kprintf("Not a child \n");
          *retval = -1;
          return -1;
        }

        
  return curthread->pidValue;
}



