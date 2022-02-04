#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/* Ready queue to keep track of threads */
queue_t ready_queue; 
/* zombie queue to keep track of finished threads */
queue_t zombie_queue;
/* pointer to current thread */
struct uthread_tcb *current;

/* TCB struct */ 
typedef struct uthread_tcb {
	uthread_ctx_t context;
	void* stack;
}uthread_tcb;

struct uthread_tcb *uthread_current(void)
{
	/* return pointer to current thread */
	return current;
}

/* queue delete function */

void uthread_yield(void)
{
	/* get current thread */
	uthread_tcb *prev = uthread_current(); 
	uthread_tcb *next = NULL;
	next = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	/* disable timer interrupt */
	preempt_disable();
	/* enqueue current thread into ready queue */
	queue_enqueue(ready_queue,prev);
	/* dequeue the ready queue to get next thread */ 
	queue_dequeue(ready_queue, (void**)&next);
	current = next;
	/* context switch */
	uthread_ctx_switch(&prev->context, &next->context);
}

void uthread_exit(void)
{
	/* get current thread */
	uthread_tcb *prev = uthread_current(); 
	uthread_tcb *next = NULL;
	next = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	/* disable timer interrupt */
	preempt_disable();
	/* enqueue current state to zombie_queue */
	queue_enqueue(zombie_queue,(void**)&prev);
	/* dequeue the ready queue to get next thread */
	queue_dequeue(ready_queue, (void**)&next);
	current = next;
	uthread_ctx_switch(&prev->context, &next->context);
}

int uthread_create(uthread_func_t func, void *arg)
{	
	/* allocate space pointing towards TCB*/
	uthread_tcb *thread = NULL;
	thread = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	/* initialize the thread and enqueue it to read queue */
	thread->stack = uthread_ctx_alloc_stack();
	uthread_ctx_init(&thread->context, thread->stack, func,arg);
	/* disable timer interrupt */
	preempt_disable();
	queue_enqueue(ready_queue, thread);
	/* enable timer interrupt */
	preempt_enable();
	return 0;
}

int uthread_start(uthread_func_t func, void *arg)
{
	/* initialize global variable ready queue */
	ready_queue = queue_create();
	zombie_queue = queue_create();
	/* set main thread to idle */
	uthread_tcb *parent = NULL;
	parent = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	/* set current thread and enqueue it to ready queue*/ 
	current = parent;
	queue_enqueue(ready_queue, parent);
	/* create the next thread */ 
	uthread_create(func, arg);
	/* starts timer interrupt */
	preempt_start();
	/* idle loop, only returns after all threads finishes */
	while(queue_length(ready_queue) > 1){
		/* enable timer interrupt each time thread runs */
		preempt_enable();
		uthread_yield();
	}
	/* destory zombie queue and free all zombie resources */
	queue_destroy(zombie_queue);
	/* resets timer */
	preempt_stop();
	return 0;
}

void uthread_block(void)
{
	/* get current thread */
	uthread_tcb *prev = uthread_current();
	uthread_tcb *next = NULL;
	next = (uthread_tcb*)malloc(sizeof(uthread_tcb));
	/* disable timer interrupt */
	preempt_disable();
	/* dequeue the ready queue to get next thread */
	queue_dequeue(ready_queue, (void**)&next);
	current = next;
	/* context switch */
	uthread_ctx_switch(&prev->context, &next->context);
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* disable timer interrupt */
	preempt_disable();
	/* enqueue the thrad received */
	queue_enqueue(ready_queue,uthread);
	/* enable timer interrupt */
	preempt_enable();
}
