#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include <private.h>

/* global variable */
int a = 0;

void thread2(void *arg)
{
	/* changes the global variable to 1 */
	a = 1;
	printf("thread2\n");
}

void thread1(void *arg)
{
	/* creates thread 2 */
	uthread_create(thread2, NULL);
	printf("thread1\n");
	/* never ending loop unless a == 1, can only be set from thread 2*/
	while(1 == 1){
		printf("waiting\n");
		if (a == 1){
			printf("going to exit\n");
			uthread_exit();
		}
	}
}

int main(void)
{
	/* start thread1 */
	uthread_start(thread1, NULL);
	/* thread 1 have finished executing */
	printf("Back in main!\n");
	return 0;
}
