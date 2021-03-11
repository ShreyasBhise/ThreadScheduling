// File:	rpthread.c

// List all group member's name: Shreyas Bhise, Ryan Jensen
// username of iLab: syb29
// iLab Server: kill.cs.rutgers.edu

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
int threadCount = 0;
int initialized = 0;
int CURR_QUEUE = 0;
int QUEUE_LEVELS = 1;
int TIMER_ENABLED = 1;
int EXIT = 0;
int YIELD = 0;
queue** queues;
queue* blocked;
queue* finished;
struct itimerval timer;

void print_queue(queue* q){
	node* n = q->front;
	while(n!=NULL){
		printf("%d ", n->TCB->tid);
		n = n->next;
	}
	return;
}

void add_front(queue* q, tcb* newTCB){
	if(q->front==NULL){
		q->front = (node*)malloc(sizeof(node));
		q->front->next = NULL;
		q->front->TCB=newTCB;
		newTCB->status=READY;
		q->back=q->front;
		return;
	}
	node* curr = q->front;
	node* newNode = (node*)malloc(sizeof(node));
	newNode->next = curr;
	newNode->TCB = newTCB;
	q->front = newNode;
	return;
}

node* pop(queue* q){
	if(q->front==NULL) return NULL;
	node* curr = q->front;
	q->front = curr->next;
	if(q->front==NULL){
		q->back=NULL;
	}
	return curr;
}

void add_back(queue* q, node* newNode){
	if(q->back==NULL){
		q->front=newNode;
		q->back=newNode;
		return;
	}
	node* prev = q->back;
	prev->next=newNode;
	q->back = newNode;
	newNode->next=NULL;
	return;
}

void timer_interrupt(int signum) {
	EXIT=0; 
	YIELD = 0;
	if(TIMER_ENABLED==0) {
		puts("bad access to timer");
	}
	TIMER_ENABLED=0;
	if(signum==69)  {
		puts("yielded");
		YIELD = 1;
	}
	else if (signum==70){
		puts("exiting thread");
		EXIT=1;
	} else if (signum==71){
		//puts("joined thread");
	} 
	//else return;
	schedule();
}

void reset_timer() {
	TIMER_ENABLED = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &timer, NULL);
	timer.it_value.tv_usec = TIMESLICE;
	setitimer(ITIMER_PROF, &timer, NULL);
}

void run_thread(void *(*function)(void*), void * arg){
	void * ret_val = function(arg);
	rpthread_exit(ret_val);
}
node* findThreadInQueue(queue* q, rpthread_t tid) {
	node* temp;
	for(temp = q->front; temp != NULL; temp = temp -> next) {
		if(temp->TCB->tid == tid) {
			return temp;
		}
	}
	return NULL;
}

node* findActiveThread(rpthread_t tid) {
	int i;
	for(i = 0; i < QUEUE_LEVELS; i++) {
		node* found = findThreadInQueue(queues[i], tid);
		if(found != NULL) return found;
	}
	//Thread not found
	return NULL;
}

node* findBlockedThread(rpthread_t tid) {
	return findThreadInQueue(blocked, tid);
}
/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	int curr_thread = 0;
	TIMER_ENABLED = 0;
    tcb* oldThread;
    if(initialized==0){
		oldThread = (tcb*)malloc(sizeof(tcb));
		oldThread->status=READY;
		getcontext(&(oldThread->context));
		if(initialized) {
			return 0;
		}
		oldThread->tid=0;
		// start scheduler
		queues = malloc(QUEUE_LEVELS*sizeof(queue*));
		int i;
		for(i = 0; i<QUEUE_LEVELS; i++){
			queues[i] = malloc(sizeof(queue));
		}
		blocked = malloc(sizeof(queue));
		finished = malloc(sizeof(queue));
		add_front(queues[QUEUE_LEVELS-1], oldThread);
		initialized = 1;
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &timer_interrupt;
		sigaction(SIGPROF, &sa, NULL);
		timer.it_interval.tv_usec = 0;
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = TIMESLICE;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
	} else {
		oldThread = queues[CURR_QUEUE]->front->TCB;
		//printf("oldThread tid: %d\n", oldThread->tid);
	}
	tcb *newThread = (tcb*)malloc(sizeof(tcb));
	getcontext(&(newThread->context));
	newThread->context.uc_link = &(oldThread->context);
	newThread->context.uc_stack.ss_sp = malloc(STK_SIZE);
	newThread->context.uc_stack.ss_size = STK_SIZE;
	newThread->context.uc_stack.ss_flags = 0;
	makecontext(&newThread->context, run_thread, 2, function, arg);
	newThread->tid = ++threadCount;
	*thread = newThread->tid;
	newThread->parent = INT_MAX;
	curr_thread = threadCount;
	//printf("numThreads: %d\n creating thread %u\n", threadCount, newThread->tid);
	add_front(queues[QUEUE_LEVELS-1], newThread);
	reset_timer();
	TIMER_ENABLED = 1;
	setcontext(&newThread->context);
    return curr_thread;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	tcb* curr = queues[QUEUE_LEVELS-1]->front->TCB;
	curr->status = READY;
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler context

	// YOUR CODE HERE
	timer_interrupt(69);
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread
	node* curr = queues[CURR_QUEUE]->front;
	tcb* thread = curr->TCB;
	add_front(finished, thread);
	thread->value = value_ptr;
	if(thread->parent != INT_MAX) {
		//Search blocked for this tid;
		//printf("thread %u is now ready\n", thread->parent);
		node* temp = findBlockedThread(thread->parent);
		temp->TCB->status = READY;
	}
	thread->parent = INT_MAX;
	timer_interrupt(70);
	return;
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	TIMER_ENABLED = 0;
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  	//Check if thread is not the same as current thread
	tcb* curr = queues[CURR_QUEUE]->front->TCB;
	//printf("Thread %d stopped until thread %d terminates\n", curr->tid, thread);
	if(thread == curr->tid)
		return -1;
	int i;
	
	node* waitFor = findActiveThread(thread);
	if(waitFor == NULL) { //Waiting for a thread that has already terminated
		if(value_ptr==NULL) return 0;
		node* temp = finished->front;
		while(temp!=NULL){
			if(temp->TCB->tid==thread){
				*value_ptr = temp->TCB->value;
			}
			temp = temp->next;
		}	
		return 0;
	}
	waitFor->TCB->parent = curr->tid;
	add_front(blocked, curr);
	curr->status=BLOCKED;
//	printf("Thread %d stopped until thread %d terminates\n", curr->tid, thread);
	timer_interrupt(71);
	if(value_ptr==NULL) return 0;
	node* temp = finished->front;
	while(temp!=NULL){
		if(temp->TCB->tid==thread){
			*value_ptr = temp->TCB->value;
		}
		temp = temp->next;
	}
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex
	rpthread_mutex_t m = *mutex;
	m.lock = 0;
	m.currThread = INT_MAX;
	m.blocked = malloc(sizeof(queue)); 

	*mutex = m;
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //  
        // context switch to the scheduler thread
		while(__sync_lock_test_and_set(&(mutex->lock), 1)) {
			node* curr = queues[CURR_QUEUE]->front;
			add_back(mutex->blocked, curr);
			curr->TCB->status = BLOCKED;
			mutex->currThread = curr->TCB->tid;
			timer_interrupt(71);
		}
		printf("end of lock:%d\n", mutex->lock);
        return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.
	printf("Queue: ");
	print_queue(mutex->blocked);
	puts("");
	mutex->currThread = INT_MAX;

	node* bThread = pop(mutex->blocked);
	while(bThread != NULL) {
		printf("unblocking thread %u", bThread->TCB->tid);
		bThread->TCB->status = READY;
		bThread = pop(mutex->blocked);
	}
	
	__sync_lock_release(&(mutex->lock));
	printf("end of unlock:%d\n", mutex->lock);
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
	sched_rr();
#else 
	// Choose MLFQ
     // CODE 2
	 sched_mlfq();
#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)
	// curr should exist, as yield cannot be called otherwise
	//print_queue(queues[0]);
	node* curr = pop(queues[0]);
	//print_queue(queues[0]);
	// Only thread in the queue
	if(curr->next==NULL){
		add_back(queues[0], curr);
		reset_timer();
		TIMER_ENABLED=1;
		return;
	}
	node* nxt = curr->next;
	//printf("EXIT=%d\n", EXIT);
	if(EXIT==0) add_back(queues[0], curr);
	curr->next=NULL;
	//finding next thread to run
	while(nxt->TCB->status==BLOCKED){
		// put nxt to the back and try the next node
		node* temp = pop(queues[0]);
		nxt=temp->next;
		add_back(queues[0], temp);
		temp->next=NULL;
	} 
	//printf("O: %d, N: %d\n", curr->TCB->tid, next->TCB->tid);
	//print_queue(queues[0]);
	//printf("\n");
	if(curr->TCB->tid==nxt->TCB->tid){ // everything else blocked, resume current
		//puts("self-switch");
		reset_timer();
		TIMER_ENABLED=1;
		return;
	}
	//printf("%d switch to %d\n", curr->TCB->tid, nxt->TCB->tid);
	//printf("%d\n", curr->TCB->status);
	reset_timer();
	TIMER_ENABLED=1;
	swapcontext(&curr->TCB->context, &nxt->TCB->context);
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

