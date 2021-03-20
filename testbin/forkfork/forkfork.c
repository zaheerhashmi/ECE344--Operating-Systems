/*
 * forktest - test fork().
 *
 * This should work correctly when fork is implemented.
 *
 * It should also continue to work after subsequent assignments, most
 * notably after implementing the virtual memory system.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int
main(int argc, char *argv[])
{
    int a;
    //int b;
    a = fork();
	//printf("this is happening1\n");
    printf("this is a: %d\n", a);
	//b = fork();
	//printf("this is happening2\n");
    //printf("this is b: %d\n", b);

    (void)argc;
    (void)argv;
    
	return 0;
}
