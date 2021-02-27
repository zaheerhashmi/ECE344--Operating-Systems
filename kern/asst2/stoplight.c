/* 
 * stoplight.c
 *
 * You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
#include <synch.h>


/*
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */

unsigned int north = 0;
unsigned int east = 1;
unsigned int south = 2;
unsigned int west = 3;

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

//Locks and CV's//
        ///Locks///
                struct lock* NW;
                struct lock* SW;
                struct lock* NE;
                struct lock* SE;
        ///Condition Vars//
                struct cv* northright;
                struct cv* eastright;
                struct cv* southright;
                struct cv* westright;
                

///Conditions///
unsigned int straight_form_north = 0;
unsigned int straight_from_south = 0;
unsigned int straight_from_east = 0;
unsigned int straight_from_west = 0;
                
static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        if(cardirection == north){}
        else if (cardirection == east)
        {
                /* code */
        }
        else if (cardirection == south )
        {
                /* code */
        }
        else if (cardirection == north)
        {
                /* code */
        }
        
        
        
        
        (void) cardirection;
        (void) carnumber;
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
                if(cardirection == north){
       
        message(APPROACHING,carnumber,cardirection,west);
        lock_acquire(NW);

        while(straight_form_north == '1'){
                cv_wait(northright,NW);}
        lock_acquire(NW);
        message(REGION1,carnumber,cardirection,west);
        message(LEAVING,carnumber,cardirection,west);
        cv_broadcast(northright,NW);
        }

                else if(cardirection == east){
        message(APPROACHING,carnumber,cardirection,north); 
        lock_acquire(NE);

        while(straight_from_east == '1'){
                cv_wait(eastright,NE);}
        lock_acquire(NE);
        message(REGION1,carnumber,cardirection,north);
        message(LEAVING,carnumber,cardirection,north);
        cv_broadcast(eastright,NE); 
        }
        
                else if (cardirection == south)
        {
         message(APPROACHING,carnumber,cardirection,east);
         lock_acquire(SE);

        while(straight_from_south == '1'){
                cv_wait(southright,SE);}
        lock_acquire(SE);
        message(REGION1,carnumber,cardirection,east);
        message(LEAVING,carnumber,cardirection,east);
        cv_broadcast(southright,SE);  
            
        }
        
                else if (cardirection == west)
        {
         message(APPROACHING,carnumber,cardirection,south);   
         lock_acquire(NE);

        while(straight_from_west == '1'){
                cv_wait(westright,SW);}
        lock_acquire(SW);
        message(REGION1,carnumber,cardirection,south);
        message(LEAVING,carnumber,cardirection,south);
        cv_broadcast(westright,SW);  
        }
        
        

        (void) cardirection;
        (void) carnumber;
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
        (void) gostraight;
        (void) turnleft;
        (void) turnright;

        /*
         * cardirection is set randomly.
         */

        cardirection = random() % 4;
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;
    
        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {
                error = thread_fork("approachintersection thread",
                                    NULL, index, approachintersection, NULL);

                /*
                * panic() on error.
                */

                if (error) {         
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error));
                }
        }
        
        /*
         * wait until all other threads finish
         */

        while (thread_count() > 1)
                thread_yield();

	(void)message;
        (void)nargs;
        (void)args;
        kprintf("stoplight test done\n");
        return 0;
}

