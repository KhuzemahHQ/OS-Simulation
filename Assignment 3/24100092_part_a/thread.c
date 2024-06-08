#include "thread.h"

// mask used to ignore some signals
// we need to ignore the alarm clock signal when we are doing critical stuff
// an empty masks means all signals are enabled
sigset_t signal_mask;

// the timer for the alarm clock
struct itimerval timer;

// stores the pointer to the currently running thread
struct TCB* running = NULL;

struct q_node* ready_q;
struct q_node* finished_q;


int num_threads = 0;

struct q_node{

    struct TCB* node_t;
    struct q_node* next;
};

struct q_node* enqueue(struct q_node *q_head,  struct TCB *node){
    
    if (q_head == NULL){
        q_head = (struct q_node*) malloc(sizeof(struct q_node));
        q_head->node_t = node;
        q_head->next = NULL;
        // printf("Done with adding head %d \n",q_head->node_t->thread_id);
    }
    else
    {
        struct q_node* temp = q_head;
        // printf("Adding to queue %d \n",node->thread_id);
        while (temp->next != NULL){
            temp = temp->next;
        }
        struct q_node* new_node = (struct q_node*) malloc(sizeof(struct q_node));
        temp->next = new_node;
        new_node->node_t = node;

    }
    // printf("Enqueue should return \n");
    return q_head;
}

struct q_node* dequeue(struct q_node *q_head){
    struct q_node* temp = q_head;

    if (temp == NULL){
        printf("Empty queue");
        return NULL;
    }
    else{
        q_head = q_head->next;
        return q_head;
    }

}

void init_lib() {

    struct q_node* ready_q = NULL;

    struct q_node* finished_q = NULL;
}

// returns id of the current thread
int self_id() {
    return running->thread_id;
}

int get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    // there was W written here after semicolon above

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

// manages the threads and decides on which thread to run next
void scheduler(int signal_number) {

    // printf("Running thread which called scheduler: %d \n",running->thread_id);
    // printf("scheduler called \n");
    if (ready_q == NULL){
        // printf("Nothing in ready queue \n");
        exit(1);    
    }
    if (signal_number == -1){

        struct TCB* old_running = running;

        struct TCB* temp = ready_q->node_t;
        ready_q = dequeue(ready_q);

        if (temp != NULL){
            running = temp;
            running->state = RUNNING;
            switch_context(old_running, running);
            // printf("After end_thread context switch \n");
        }
        else{
            printf("Ready queue not correctly pointing \n");
            exit(1);
        }
    }
    else{
        // printf("Interrupt \n");
        struct TCB* old_running = running;
        running->state = READY;
        ready_q = enqueue(ready_q,running);
        struct TCB* temp = ready_q->node_t;
        ready_q = dequeue(ready_q);

        if (temp != NULL){
            
            running = temp;
            running->state = RUNNING;

            switch_context(old_running, running);
            // printf("After context switch \n");
        }
        else{
            printf("Ready queue not correctly pointing \n");
            exit(1);
        }
    }

}

addr map_address(addr address) {
	addr ret;
	asm volatile("xor    %%fs:0x30,%0\n"
			"rol    $0x11,%0\n"
			: "=g" (ret)
			  : "0" (address));
	return ret;
}

void enable_interrupts() {
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
}

void disable_interrupts() {
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);
}

void timer_start() {
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_SLICE;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_SLICE;

    // when a time slice is over, call the scheduler function
    // by raising the SIGALRM signal
    signal(SIGALRM, scheduler);

    // this mask will be used to mask the alarm signal when we are
    // performing critical tasks that cannot be left midway
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGALRM); // add only the alarm signal

    // start the timer
    setitimer(ITIMER_REAL, &timer, NULL);
}

void timer_stop() {
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
}

int create_thread(void (*callback)) {

    // printf("Create thread called \n");
    
    struct TCB* thread = (struct TCB*) malloc(sizeof(struct TCB));

	//Thread ID
	thread->thread_id = ++num_threads;

    // Allocating the stack
    thread->stack = (char*) malloc(STACK_SIZE);
    thread->stack_size = STACK_SIZE;
    thread->sp = (addr)thread->stack + STACK_SIZE - sizeof(int); 

    // setting the Program Counter to the thread callback_routine
    thread->pc = (addr) callback;
    thread->callback_routine = callback;
    thread->waiting_id = -1;    // no thread is waiting on it. used in the case of the "join" function

    if (num_threads == 1) {
        // this means we are creating the first thread, that is the main thread
        running = thread;
        running->state = RUNNING;
        // printf("Done with creating first thread \n");
    }
    else {
        // set the PC and SP registers to the address of the function we passed
        sigsetjmp(thread->jbuf, 1);
        (thread->jbuf->__jmpbuf)[JB_SP] = map_address(thread->sp);
        (thread->jbuf->__jmpbuf)[JB_PC] = map_address(thread->pc);
        sigemptyset(&thread->jbuf->__saved_mask);

        thread->state=READY;
        
        /*
        Task1: Add this thread to the ready queue
        */
    //    printf("Before enqueing to ready \n");
       ready_q = enqueue(ready_q, thread);
    //    printf("Done with enqueing to ready \n");
       
    }
    
	return thread->thread_id;
}

void end_thread() {
    disable_interrupts();
    running->state = FINISHED;
    --num_threads;

    /*
    Task1: Add this thread to the finished_queue
    */

   finished_q = enqueue(finished_q, running);
//    printf("Done with enqueing %d to finished \n",running->thread_id);

    // passing -1 as it is not being called as a result of an interrupt
    // and we are calling it manually
    scheduler(-1);
}

void switch_context(struct TCB* old_thread, struct TCB* new_thread) {
    // reset the timer for the new process that is going to run
    // so that i receives its full time slice
    setitimer(ITIMER_REAL, &timer, NULL);

    // save the context of the current running thread
    int ret_val = sigsetjmp(old_thread->jbuf, 1);
    if (ret_val == 1) {
        enable_interrupts();
        return;
    }

    enable_interrupts();
    // switch to the next thread
    siglongjmp(new_thread->jbuf, 1);
}


void sleep(int milliseconds){

    long long mill = get_time();
    long long diff = get_time();

    while (diff<mill+milliseconds){
        diff = get_time();
    }

}

void yield(){
    // printf("%d is yielding \n",running->thread_id);
    // ready_q = enqueue(ready_q, running);

    scheduler(2);
}
