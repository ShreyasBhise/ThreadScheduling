// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
int threadCount = 0;
int initialized = 0;
int CURR_QUEUE = 0;
int QUEUE_LEVELS = 1;
int TIMER_ENABLED = 1;
queue** queues;

void add_front(queue* q, tcb* newTCB){
	if(q->front==NULL){
		q->front = (node*)malloc(sizeof(node));
		q->front->next = NULL;
		q->front->TCB=newTCB;
		q->back=q->front;
		return;
	}
	node* curr = q->front;
	node* newNode = (node*)malloc(sizeof(node));
	newNode->next = curr;
	newNode->TCB = newTCB;
	q->front = newNode;
}

void timer_interrupt(int signum) {
	if(!TIMER_ENABLED) return;
	if(signum==69) puts("yielded");
	else puts("signum: %d", signum);
	schedule();
}

void reset_timer() {
	TIMER_ENABLED = 0;
	struct itimerval timer;
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &timer, NULL);
	timer.it_interval.tv_usec = TIMESLICE;
	timer.it_value.tv_usec = TIMESLICE;
	setitimer(ITIMER_PROF, &timer, NULL);
}

/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	TIMER_ENABLED = 0;
    tcb* oldThread;
    if(initialized==0){
		oldThread = (tcb*)malloc(sizeof(tcb));
		getcontext(&(oldThread->context));
		// start scheduler
		queues = malloc(QUEUE_LEVELS*sizeof(queue*));
		int i;
		for(i = 0; i<QUEUE_LEVELS; i++){
			queues[i] = malloc(sizeof(queue));
		}
		add_front(queues[QUEUE_LEVELS-1], oldThread);
		initialized = 1;
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &timer_interrupt;
		sigaction(SIGPROF, &sa, NULL);
		struct itimerval timer;
		timer.it_interval.tv_usec = TIMESLICE;
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = TIMESLICE;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
	} else {
		oldThread = queues[CURR_QUEUE]->front->TCB;
	}
	tcb *newThread = (tcb*)malloc(sizeof(tcb));
	getcontext(&(newThread->context));
	newThread->context.uc_link = &(oldThread->context);
	newThread->context.uc_stack.ss_sp = malloc(STK_SIZE);
	newThread->context.uc_stack.ss_size = STK_SIZE;
	newThread->context.uc_stack.ss_flags = 0;
	makecontext(&newThread->context, function, arg);
	newThread->tid = ++threadCount;
	add_front(queues[QUEUE_LEVELS-1], newThread);
	setcontext(&newThread->context);
	reset_timer();
	TIMER_ENABLED = 1;
    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
	timer_interrupt(69);
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //  
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (RR or MLFQ)

	// if (sched == RR)
	//		sched_rr();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose RR
     // CODE 1
#else 
	// Choose MLFQ
     // CODE 2
#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

