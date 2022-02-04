#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

void test_queue_enqueue_dequeue()
{
	fprintf(stderr, "*** TEST queue_enqueue_dequeue ***\n");
	/* Tests for correct enqueue and dequeue of various data types */
	int * i_ptr;
	char * c_ptr;
	long * l_ptr;
	int data_1 = 5;
	char data_2 = 'w';
	long data_3 = 25;
	queue_t q;
	q = queue_create();
	/* enqueue data 5, 'w', and 25 */
	queue_enqueue(q, &data_1);
	queue_enqueue(q, &data_2);
	queue_enqueue(q, &data_3);
	/* Tests if dequeues 5 */
	queue_dequeue(q, (void**)&i_ptr);
	TEST_ASSERT(i_ptr == &data_1);
	/* Tests if dequeues 'w' */
	queue_dequeue(q, (void**)&c_ptr);
	TEST_ASSERT(c_ptr == &data_2);
	/* Tests if dequeues '25' */
	queue_dequeue(q, (void**)&l_ptr);
	TEST_ASSERT(l_ptr == &data_3);
	TEST_ASSERT(l_ptr == &data_3);
	TEST_ASSERT(queue_length(q) == 0);
}

 queue_t q;

/* Callback function that increments items */
static void inc_item(void *data) {
	int *a = (int*)data;
	if (*a == 5){
		queue_delete(q, data);
	}
	else{
		*a += 1;
	}
}

void test_iterator(void) {
	fprintf(stderr, "*** TEST iterator ***\n");
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; 
	int i;
    
    	/* Initialize the queue and enqueue items */
	q = queue_create();
	
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	
	/* Increment every item of the queue, delete item '5' */
	queue_iterate(q, inc_item);
	TEST_ASSERT(data[0] == 2); 
	TEST_ASSERT(queue_length(q) == 9);
}


void test_delete(void) {
	queue_t q;
	fprintf(stderr, "*** TEST delete ***\n");
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; 
	int i;
    
    	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	/* delete data[5] twice, delete data[4] once */
	queue_delete(q, &data[5]);
	queue_delete(q, &data[5]);
	queue_delete(q, &data[4]);
	for (i = 0; i < 9; i++){
		printf("%d", data[i]);
	}
	/* See if data[5] and data[4] are actually deleted. */
	int * i_ptr;
	queue_dequeue(q, (void**)&i_ptr);
	TEST_ASSERT(i_ptr == &data[0]);
	queue_dequeue(q, (void**)&i_ptr);
  	TEST_ASSERT(i_ptr == &data[1]);
	queue_dequeue(q, (void**)&i_ptr);
  	TEST_ASSERT(i_ptr == &data[2]);
	queue_dequeue(q, (void**)&i_ptr);
  	TEST_ASSERT(i_ptr == &data[3]);
	queue_dequeue(q, (void**)&i_ptr);
  	TEST_ASSERT(i_ptr == &data[6]);
	queue_dequeue(q, (void**)&i_ptr);
  	TEST_ASSERT(i_ptr == &data[7]);
	/* only 2 elements left */
	TEST_ASSERT(queue_length(q) == 2);
	/* try delete again */
	queue_delete(q, &data[8]);
	TEST_ASSERT(queue_length(q) == 1);
	queue_dequeue(q, (void**)&i_ptr);
	TEST_ASSERT(i_ptr == &data[9]);
}


void test_delete_head(void) {
	queue_t q;
	fprintf(stderr, "*** TEST delete ***\n");
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; 
	int i;
    
    	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);
	/* delete the head of the queue */
	queue_delete(q, &data[0]);
	int * i_ptr;
	/* test out to see if queue is still in order */
	queue_dequeue(q, (void**)&i_ptr);
	TEST_ASSERT(i_ptr == &data[1]);
	/* test out the length */
	TEST_ASSERT(queue_length(q) == 8);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_enqueue_dequeue();
	test_delete();
	test_iterator();
	test_delete_head();
	return 0;
}
