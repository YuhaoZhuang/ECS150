# Report for Project #2 - User-level thread library

## Phase 0: Makefile
The makefile for the libuthread static library is designed to be portable, and
automate the build process of our project. This file contains conditional
variables, which provide the user with command line compilation options for
@ -13,85 +12,21 @@ finally has the option to clean the libuthread.a, object
files, and dependency
files.

## Phase 1: queue API
To implement a simple FIFO queue, with all operations constrained to O(1)
complexity(except iterate and delete operations), a struct of linked list is
used. The linked list struct has an integer to keep track of how many nodes
there are inside the linked list, and it also contains a pointer to the first
node and last node of the list. The first node will represent the first or the
earliest item in the queue, and the tail node represents the most recent item
in the queue. Each node is a struct that has a node pointer, which points
towards the next node, and a void* data holder storing the address of the data
received. To initialize the queue, queue_create() is called and it allocates
the memory needed for a new queue and returns a pointer towards it.
Queue_length() returns the integer stored inside the struct for queue,
therefore the length of the queue. Queue_enqueue() enqueues a new node by
alloacting memory for it and setting the value of data pointer in the node to
the value the function receives. If the queue is empty, then both head and tail
pointer would point to the node that was just set. If the queue is not empty,
it would set the tail node's next node pointing towards the current node, and
then it would set the tail node to the current node. In both cases, the queue's
length would then increment by 1. Queue_dequeue() receives a double pointer of
type void and it copies the value of data pointer stored in the queue's first
node to that double pointer. It then decrements the queue's length by 1 and
sets the head node to head's next node or the current node. At the end, it
returns the changed double pointer back to its caller, so the caller can access
the data stored in the first item/head in the queue. Queue_delete() receives a
data pointer and searches the queue to find the first node that has the same
value of data pointer and delete that node. It does so by creating 2 nodes,
first node storing the previous node, second node stores the current node. If
the head node of the queue has the same data has the data pointer received,
then the head node will be set to the next node. Otherwise, the queue iterates
through every single node of the queue and compares the value of their data
pointers to the one received by the function. The previous node will be set to
the current node and the current node will be set to the next node it points
to, therefore continuing the iteration until no more nodes left. Once the node
with the same value of data pointer is found, the loop breaks and sets the
pervious node's next node to current node's next node, therefore breaks the
current node's link in the queue. In either case whether the head node contains
the same value of data pointer as the one received in function or not, the
second node, which serves a copy of the current node, will be freed, and the
length of queue is decremented by 1. Queue_iterate() receives a function
pointer and calls the function on every single node of the queue. It does so by
creating 2 nodes, first node set to the head node of queue, second node set to
null. The function then starts a while loop which iterates on the first node
until it reaches the end of the queue. In each itereation, the second node will
be set to the first node's next node, so it serves as a copy of the next node
of the current/first node in case the current/first node gets destoryed in the
subsequent function call. The function pointer received by the queue_iterate
will be then called on the current node, and after the execution of the
function, the current/first node will be set to the second node, which was
saved previously. Both nodes are freed at the end of the function.
Queue_destory can be called to free up the memory allocated by the queue, thus
destorying it. 

## Phase 1: testing
To test out the correctness of the queue API, test cases are created. Example
testers from the project prompt are incorporated. Queue_create() is tested by
verfiying the data returned isn't NULL. Queue_enqueue() and Queue_dequeue() are
tested by enqueueing mutiple times with different types of variables, and then
dequeue them to test out if the variables returned are the ones enqueued in the
correct order. The length of the struct is also verfied at the end to test out
queue_length(). Queue_iterate() is tested by iterating through a queue of
nodes, calling queue_delete on one of them, which test out if queue_iterate
will fail if one of the node gets deleted in the middle, and it then increments
all the values by 1 and checks if the values are indeed incremeneted.
Queue_delete() is tested by having multiple deletes on the same node, and then
dequeue the rest of the list to see if the multiple delete on the same node has
caused any errors, and if the queue has the right order. It then compares the
length of the queue to see if it is the correct value. Another special case of
queue_delete, which is the deletion of head pointer is tested by simply
deleting the first/head node of the queue, the order of the queue and the
length of the queue is then checked to ensure the correctness.

## Phase 2: uthread API
For the uthread implementation, a "thread control block"/TCB struct is used to
contain the context and the stack pointer of the thread. A global variable
ready queue is declared using the queue API implemented in phase 1 to keep a
list of threads, another global variable zombie_queue is declared to collect
the TCBs of the threads that have finished executing, and a global variable
pointing towards the TCB is declared to keep track of the current threads. To
start the threading process, uthread_start() should be called and it
initializes those 3 global variables. It then creates a TCB of the current
process as the idle thread, and that idle thread is then pushed to the ready
queue. After that, the function calls uthread_create() on the func and arg
given, and uthread_create() creates a new thread. Uthread_create() creates a
struct TCB by alloacting its stack pointers and initialize this new thread and
push it to the ready queue. Back to uthread_start(), the function starts a
while loop, which keeps iterating until there are no more threads besides the
idle thread itself is left in the ready queue. In each iteration of the while
loop, uthread_yield() is called to yield the execution of the idle process.
Uthread_yield() stops the execution of the current thread and switches to the
next available thread in the ready queue. It does so by calling
uthread_current(), which returns the global variable that pooints to the
current thread. uthread_yield() enqueues this current thread and pushes this
thread to the end of the ready queue, and then it dequeues a thread from the
ready queue and sets the current thread to be that dequeued thread. It then
calls uthread_ctx_switch() to switch the threading process from the enqueued
thread to the dequeued thread. So in the while loop of uthread_start(), it
calls uthread_yield() in each iteration thus giving away the resource for
executing process to any other threads that's left in the ready queue. After
all the other threads in the ready queue have been dequeued, meaning all those
threads have finished executing, the while loop will then end and execute the
idle thread's content. Uthread_exit() is called everytime the threads are
scheduled to run for the first time and completely finishes their execution.
Since the thread has finished executing, it pushes this thread to the zombie
queue. Then it dequeues the next thread in the ready queue, sets it to the
current thread, and switches the thread exection process to that dequeued
thread. At the end of uthread_start(), the zombie queue will be destoryed and
all the TCB of the finished threads will be freed.

## Phase 3: semaphore API
To implement the semaphore, a semaphore struct is defined containing an
internal count to keep track of available resources, and it also contains a
wait queue using the queue API from phase 1 to contain all the waiting threads.
To initialize the semaphore, sem_create() is called and it creates a new
semaphore struct varaiable, which has the internal count set to the integer
passed to the function argument and an empty waiting queue. Sem_create() then
returns this semaphore variable for processes to manipulate it. When
sem_destory() is called on the semaphore varaible, it checks if the internal
count of the semaphore is 0. If it is not 0, the internal count would decrement
by 1. If it is 0, then the current thread will be enqueued to the waiting
thread in the semaphore, and uthread_block() will be called. Uthread_block()
calls uthread_current() to get the current thread, dequeues a thread from the
ready queue, set the current thread to that dequeued thread, and switches its
context to execute the dequeud thread. Bascially exeecuting the next available
thread in the ready queue. Since the thread calling sem_down() is enqueued into
the waiting queue and no longer belongs to the ready queue, the thread will not
get executed unless it is unblocked. Sem_up() can be called on the semaphore
struct variable to undo the blocking process. Sem_up() always increments the
internal count of the semaphore by 1, then it checks if there are any threads
in the waiting queue. If there are, then sem_up()decreemnts the interal count
of the semaphore, dequeues the waiting thread thus taking out the first
available thread that's being blocked, and it calls uthread_unblock() on the
dequeued waithing thread. Uthread_unblock() enqueues the waiting thread into
the ready queue, bascially unblocking the blocked thread by sending it back to
the ready queue.

## Phase 4: preemption
To enable preemptive scheduling, 4 global variables are declared. A struct
sigaction for handling interrupts; 2 struct itimerval for setting the timer
interrupt, one for triggering the interrrupts, the other one for saving the
previous states of the timer; and a sigset_t variable for disabling and
enabling the interrupts. To start the process of interruption, preempt_start()
is called and it initializes the sigset_t variable and adds SIGVTALRM so the
program can decide to block or unblock the timer interrupt later on. The struct
sigaction variable is initialized and set with the uthread_yield() function for
handling any interrupts from the SIGVTALRM timer interrupt signal. The current
state of the timer will then be saved onto one of the itimerval variable
declared earlier for back-ups, and the values of the timer will then be set to
trigger every 10 miliseconds or 0.01 seconds since the polling rate is 100 Hz.
The preempt_start() will be called before the while loop of the
uthread_create() function to set up all the variables needed for timer
interrupts, interrupt handling, and late interrupt blocking. After the while
loop of the uthread_create() finishes, uthread_start() will then call
preempt_stop(), which resets the value of the timer struct to the one
preempt_start() has saved earlier, and thus restoring the timer struct back to
its original state. To disable the timer interrupts when the program is
accessing the critical section, which is the dequeuing and enqueuing of the
ready queue and the modifying of the current thread, preempt_disable() is used.
Preempt_disable() sets the sigset_t global variable initialized in the
preempt_start() to SIG_BLOCK, which will block all the SIGVTALRM signals. To
enable the interrupts again, preempt_enable() is called, and it sets the
sigset_t to SIG_UNBLOCK, which enables all the SIGVTALRM singals. The
preempt_enable() is called whenever the thread is choosen to run for the first
time to make sure that the thread can be interrupted by the timer. In phase 2,
right before whenever the function is dequeuing/enqueuing the queue and
modfying the current thread, preempt_disable() is called. If the functions
continues without context switching, then preempt_enable() will be called
before the function returns to ensure that the other functions can be
interrupted by the timer. If the functions continues with context switching,
then context switching itself is also part of the critical section, and
preempt_enable() needs to be called at inside the while loop of uthread_start()
before the idle thread yields to the next available thread to ensure that the
context switching doesn't get interrupted and the other threads can be
interrupted again. In phase 3, whenever the functions are accessing the global
semaphore struct and editing its count or dequeuing/enqueuing its wait queue,
preempt_disable() and preempt_enable() will be wrapped around those executions
to ensure they don't get interrupted.

## Phase 4 : testing
The testing for preemption is a simple program designed only to return to main
in the case that preemption works successfully. A global variable is declared
as a count. The main program will start a thread, called 1, which creates
another thread, thread 2, immediately but then enters a while loop. The while
loop will only return if the global variable is modified in thread 2. So if
timer interrupt is disabled or incorrect, thread 2 will never be able to run,
and thread1 will be keep while looping. If the program does finish executing
instead of hanging there in a while loop, it means that the timer interrupt is
correctly fired, and the program is taking the appropriate signal handler to
preempt the never ending while loop in thread1. 
