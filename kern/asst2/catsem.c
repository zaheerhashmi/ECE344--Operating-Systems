/*
 * catsem.c
 *
 * Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include "catmouse.h"
#include <synch.h>
#include <machine/spl.h>


/*GLOBALS*/
struct semaphore* cat;
struct semaphore* mouse;
struct semaphore* _bowl;
struct semaphore *bowl_1; 
struct semaphore *bowl_2;
struct semaphore *condstatus; 
// bowl_status: 0 means free to use; 1 means occupied// 
int bowl_status[2]= {0,0}; 
//int cat_flag = 0;




/*
 * 
 * Function Definitions
 * 
 */

/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
        
        int spl = splhigh();
        if(mouse->count == 0){
        splx(spl);
        thread_yield(); }

        P(_bowl); // Get the bowl 
        P(cat);
        int meals = 0;
        //cat_flag = 1;
        while(meals < NMEALS)
        {
                if(bowl_status[0]==0)
                  {//P(bowl_1);
                   bowl_status[0] = 1;
                   catmouse_eat("cat",catnumber,1,meals);
                   meals++;
                   bowl_status[0] = 0;
                 // V(bowl_2);    
                 }

                if(bowl_status[1] ==0 ){
                 // P(bowl_2);
                   bowl_status[1] = 1;
                   catmouse_eat("cat",catnumber,2,meals);
                   meals++;
                   bowl_status[1] = 0;
                 // V(bowl_2);
                }
     
        }
        
        V(cat);
        V(_bowl);
      //  cat_flag = 0;
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{       
        // Check if a cat is eating if it is dont approach bowl; no cat is eating go to bowl//

        int spl = splhigh();
        if(cat->count == 0){
        splx(spl);
        thread_yield(); }
        
        
                P(_bowl);
                P(mouse);

                int meals = 0;
                while(meals < NMEALS) {
           
             if(bowl_status[0] == 0){
                //P(bowl_1);
                bowl_status[0] = 1;
                catmouse_eat("mouse",mousenumber,1,meals);
                meals++;
                bowl_status[0] = 0;
              //  V(bowl_1);
            }
          

             if( bowl_status[1] == 0)
             {            
             //  P(bowl_2);     
                bowl_status[1] = 1;
                catmouse_eat("mouse",mousenumber,2,meals);
                meals++;
                bowl_status[1] = 0;
             //  V(bowl_2);
             }
  



        }
        V(mouse);
        V(_bowl);
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) mousenumber;
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;

        cat = sem_create("cat",6);
        mouse = sem_create("mouse",2);
        _bowl = sem_create("bowl",2);
        bowl_1= sem_create("bowl_1",1);
        bowl_2 = sem_create("bowl_2",1);
        condstatus = sem_create("condstatus",1);
   
        /*
         * Start NCATS catsem() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * wait until all other threads finish
         */

        while (thread_count() > 1)
                thread_yield();

        (void)nargs;
        (void)args;
        kprintf("catsem test done\n");

        return 0;
}

