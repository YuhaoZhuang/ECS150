#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include "queue.h"

/* node for singly linked list*/
typedef struct Node {
	void *data; 
    struct Node *next; 
}Node; 

/* a queue using singly linked list, has pointer to front and end of queue */
typedef struct queue{
	int count;
	Node *front;
	Node *rear;
}queue;

queue_t queue_create(void)
{
	/* Create pointer to queue object*/
	queue_t q = NULL;
	/* Allocate memory for new queue*/
	q = (queue*)malloc(sizeof(queue));
	/* Verify mem for new queue was allocated*/
	if(q == NULL){
		/* Return 0 if allocation failed*/
		return 0;
	}
	/* Otherwise, initialize queue. */
	q->front = q->rear = NULL;
	q->count = 0;
	/* Return address to new queue structure*/
	return q;
}

int queue_destroy(queue_t queue)
{
	/* check to see if queue is NULL or empty */
	if(queue == NULL || queue->count != 0){
		return -1;
	} else {
		/* free up the space */
		free(queue);
		return 0;
	}
}

int queue_enqueue(queue_t queue, void *data)
{
	/* alloacting memory for a single node */
	Node *node;  
	node = (Node*)malloc(sizeof(Node));
	/* assigning data to node */
	node->data = data;
	node->next = NULL;
	/* If queue is NULL or data is NULL */
	if (queue == NULL || data == NULL){
		return -1;
	}
	/* If queue is empty */
	if (queue->front == NULL){
		/* assign both queue's front and end to node */
		queue->front = node;
		queue->rear = node;
	} else {
		/* add a node at queue's end */
		queue->rear->next = node;
		queue->rear = queue->rear->next;
	}
	/* increment count */
	queue->count += 1;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	/* check to see if queue or data is null or if it is empty */
	if (queue->front == NULL || data == NULL || queue->count == 0){
		return -1;
	}
	/* assign value of the first element of queue to data */
	*data = queue->front->data;
	/* move queue's head pointer to the next node and decement count*/
	queue->front = queue->front->next;
	queue->count -= 1;
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	/* Return if queue or data is NULL */
	if(queue == NULL || data == NULL){
		return -1;
	}
	/* save a copy of the queue's first node */ 
	Node *temp = queue->front;
	Node *prev = NULL;
	/* if the earlierst stored item/first node's data = data received*/
	if (temp != NULL && temp->data == data){
		/* move front to the next, decerement count, and free the temp var */
		queue->front = temp->next;
		queue->count -= 1;
		free(temp);
		return 0;
	}
	/* while the node's data does not equal to data received */
	while (temp != NULL && temp->data != data){
		/* save a copy of current node and move on the the next node */
		prev = temp;
		temp = temp->next;
	}
	/* if no matching data is found */
	if (temp == NULL){
		return -1;
	} else {
		/* decerement count by 1 */
		queue->count -= 1;
		/* point previous ndoe's next node to current node's next node*/
		prev->next = temp->next;
		/* destory the temp ndoe */
		free(temp);
		return 0; 
	}
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	/* if queue or function is null  */
	if(queue == NULL || func == NULL){
		return -1;
	} else {
		/* save a copy of current node in case it gets deleted */
		Node *node;
		Node *temp;
		node = queue->front;
		/* iteration for all nodes */
		while(node != NULL){
			/* saves a copy of the next node in case current node */
			/* gets destoreyd */
			temp = node->next;
			/* function call */
			func(node->data);
			/* set node to the previous saved */
			node = temp;
		}
		/* free the nodes */
		free(temp);
		free(node);
		return 0;
	}
}

int queue_length(queue_t queue)
{
	/* if queue is NULL */
	if(queue == NULL){
		return -1;
	} else {
		/* return length of queue */
		return queue->count;
	}
}
