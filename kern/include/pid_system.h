#ifndef _PID_H_
#define _PID_H_

struct pid { 
    int pidValue;
    int isUsed;     //1 means already used; 0 means available
    struct pid* next; 
}; 

//creates a new pid list with the first pidValue of 1
struct pid* create_pid_list();

//add a new pid to the end of the list
//used in assign_pid
void append_pid(struct pid* head, int pidValue);

/*  changes appropriate pid isUsed to 1 and returns
    which pid was modified. It will either modify a pid 
    in the middle of the list changing isUsed from 0 to 1
    or it will append a new pid and set that isUsed to 1
*/
int assign_pid(struct pid* head);

//changes isUsed to 0 for pid with pidValue
//if pidValue given does not exist, nothing happens
void release_pid(struct pid* head, int pidValue);

//kfrees every dynamically allocated pid in the list
void delete_pid_list(struct pid* head);

#endif /* _PID_H_ */
