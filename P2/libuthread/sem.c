#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"
#include "uthread.h"

typedef struct semaphore {
	/* Semaphore count*/
	int count;
	/* Queue for waiting threads*/
	queue_t wait_queue;
}semaphore;

sem_t sem_create(size_t count)
{
	/* initialize a semaphore struct */
	preempt_disable();
	sem_t my_sem = malloc(sizeof(semaphore));
	/* return 0 if memory allocation failure */
	if (my_sem == NULL){
		return 0;
	}
	my_sem->count = count;
	my_sem->wait_queue = queue_create();
	preempt_enable();
	/* return semaphore */
	return my_sem;
}

int sem_destroy(sem_t sem)
{
	/* return -1 if sem is null or other threads still blocked */
	if (sem == NULL || queue_length(sem->wait_queue) != 0){
		return -1;
	}
	/* free up semaphore memory */
	free(sem);
	return 0;
}

int sem_down(sem_t sem)
{
	
	/* return -1 if sem is null */
	if (sem == NULL){
		return -1;
	}
	/* if semaphore = 0 */
	if (sem->count == 0){
		/* enqueue the current process into waiting queue */
		preempt_disable();
		queue_enqueue(sem->wait_queue,uthread_current());
		preempt_enable();
		/* block the thread */
		uthread_block();
	} else {
		/* decrement semaphore */
		preempt_disable();
		sem->count -=1; 
		preempt_enable();
	}
	
	return 0;
}

int sem_up(sem_t sem)
{
	/* return -1 if sem is null */
	if (sem == NULL){
		return -1;
	}
	/* increment semaphore */
	preempt_disable();
	sem->count += 1;
	preempt_enable();
	/* if there are blocked threads in waiting queue */
	if (queue_length(sem->wait_queue) > 0){
		void* next;
		preempt_disable();
		/* decrement semaphore */
		sem->count -= 1;
		/* dequeue the blocking thread and unblock it */
		queue_dequeue(sem->wait_queue, (void**)&next);
		preempt_enable();
		uthread_unblock(next);
	}
	return 0;
}
