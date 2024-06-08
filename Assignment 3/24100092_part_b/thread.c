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
struct q_node* blocked_q;


int num_threads = 0;

struct q_node* enqueue(struct q_node *q_head,  struct TCB *node){
    
    if (q_head == NULL){
        q_head = (struct q_node*) malloc(sizeof(struct q_node));
        q_head->node_t = node;
        q_head->next = NULL;
    }
    else
    {
        
        if (q_head->node_t->priority >= node->priority){
            // printf("Adding to head \n");
            struct q_node* new_node = (struct q_node*) malloc(sizeof(struct q_node));
            new_node->node_t = node;

            new_node->next = q_head;
            q_head = new_node;
        }

        else{

            struct q_node* temp = q_head;
            while (temp->next != NULL && temp->next->node_t->priority < node->priority){
                temp = temp->next;
            }
            struct q_node* new_node = (struct q_node*) malloc(sizeof(struct q_node));
            new_node->node_t = node;

            new_node->next = temp->next;
            temp->next = new_node;
            
        }

    }
    return q_head;
}

struct TCB* dequeue(struct q_node *q_head){
    struct q_node* temp = q_head;
    struct q_node* previous;
    
    if (temp == NULL){
        return NULL;
    }

    if(temp->next == NULL){
        q_head = NULL;
        return temp->node_t;
    }

    else{
        while (temp->next != NULL) 
        {
            previous = temp; 
            temp = temp->next; 
        }
        previous->next = NULL;
        return temp->node_t;
    }
}


struct q_node* antistarve(struct q_node *q_head){
    struct q_node* temp = q_head;
    if (temp == NULL){
        return NULL;
    }
    while (temp->next != NULL ){
        
        if (temp->node_t->thread_id != 1){
            temp->node_t->priority = temp->node_t->priority + 1;
        }
        temp = temp->next;
    }
    if (temp->node_t->thread_id != 1){
        temp->node_t->priority = temp->node_t->priority + 1;
    }
    return q_head;
}

void printpri(struct q_node* q_head){
    printf("Printing priorities \n");
    if (q_head == NULL){
        printf("empty queue \n");
        return;
    }

    struct q_node* temp = q_head;
    while (temp->next != NULL ){
        printf("Thread %d with priority %d \n",temp->node_t->thread_id, temp->node_t->priority);
        temp = temp->next;
    }
    printf("Thread %d with priority %d \n",temp->node_t->thread_id, temp->node_t->priority);

}

struct q_node* findnode(struct q_node* q_head, int findid){

    struct q_node* temp = q_head;
    if (q_head == NULL){
        return NULL;
    }
    struct q_node* result = NULL;
    while (temp->next != NULL){
        if (temp->node_t->thread_id == findid){
            result = temp;
        }
        temp = temp->next;
    }
    if (temp->node_t->thread_id == findid){
        result = temp;
    }
    return result;
}

struct TCB* removenode(struct q_node* *q_head, int findid){

    if (*q_head == NULL){
        return NULL;
    }
    
    if((*q_head)->node_t->thread_id == findid){
        struct q_node* temp = *q_head;
        *q_head = (*q_head)->next;
        return temp->node_t;
    }

    struct q_node* temp;
    struct q_node *current  = *q_head;
    while(current->next != NULL)
    {
        if(current->next->node_t->thread_id == findid)
        {
            temp = current->next;
            current->next = current->next->next;
            return temp->node_t;
        }
        else{
            current = current->next;
        }
    }
    if(current->node_t->thread_id == findid)
    {
        temp = current->next;
        current->next = current->next->next;
        return temp->node_t;
    }

    return NULL;

}

void init_lib() {

    struct q_node* ready_q = NULL;
    struct q_node* finished_q = NULL;
    struct q_node* blocked_q = NULL;
}

// returns id of the current thread
int self_id() {
    return running->thread_id;
}

int get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

// manages the threads and decides on which thread to run next
void scheduler(int signal_number) {

    if (ready_q == NULL){
        exit(1);    
    }
    if (signal_number == -1){

        struct TCB* old_running = running;
        struct TCB* temp = dequeue(ready_q);

        running = temp;

        running->state = RUNNING;

        switch_context(old_running, running);

    }
    else{

        struct TCB* old_running = running;
        running->state = READY;
        ready_q = enqueue(ready_q,running);
        
        struct TCB* temp = dequeue(ready_q);
        
        running = temp;
        running->state = RUNNING;

        ready_q = antistarve(ready_q);

        switch_context(old_running, running);
        
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

int create_thread(void (*callback),int pri) {
    
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
    thread->priority = pri;

    if (num_threads == 1) {
        // this means we are creating the first thread, that is the main thread
        running = thread;
        running->state = RUNNING;
    }
    else {
        // set the PC and SP registers to the address of the function we passed
        sigsetjmp(thread->jbuf, 1);
        (thread->jbuf->__jmpbuf)[JB_SP] = map_address(thread->sp);
        (thread->jbuf->__jmpbuf)[JB_PC] = map_address(thread->pc);
        sigemptyset(&thread->jbuf->__saved_mask);

        thread->state=READY;
        
       ready_q = enqueue(ready_q, thread);
       
    }
    
	return thread->thread_id;
}

void end_thread() {
    disable_interrupts();
    running->state = FINISHED;
    --num_threads;

    finished_q = enqueue(finished_q, running);
    struct TCB* en = removenode(&ready_q,running->thread_id);

    if (running->waiting_id != -1 && blocked_q != NULL){
        struct TCB* blo = removenode(&blocked_q,running->waiting_id);
        
        if (blo != NULL){
            blo->state = READY;
            ready_q = enqueue(ready_q,blo);

        }
        else{
            printf("Couldn't find in blocked. \n");
            printpri(blocked_q);
        }
    }

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

    scheduler(2);
}

void block(){

    running->state = BLOCKED;

    blocked_q = enqueue(blocked_q, running);

    scheduler(-1);
}

void join(int thread_id){
    // Search for node with this thread
    if (thread_id == self_id()){
        printf("Can't join with yourself \n");
        return;
    }
    
    struct q_node* target = findnode(ready_q,thread_id);

    if (target == NULL){
        target = findnode(blocked_q,thread_id);
        if (target == NULL){
            return;
        }
    }
    
    target->node_t->waiting_id = self_id();
    block();
}

void sem_init(struct Semaphore* sem, int value){
    sem->value = value;
    sem->waitlist = NULL;
}

void sem_wait(struct Semaphore* sem){

    if (sem->value > 0){
        sem->value --;
    }
    else{
        sem->waitlist = enqueue(sem->waitlist,running);
        struct TCB* en = removenode(&ready_q,running->thread_id);
        block();
    }
    
}

void sem_post(struct Semaphore* sem){

    sem->value++;
    
    struct TCB* waiter = dequeue(sem->waitlist);
    
    if (waiter != NULL){

        struct TCB* blo = removenode(&blocked_q,waiter->thread_id);

        if (blo != NULL){
            blo->state = READY;
            ready_q = enqueue(ready_q,blo);
        }
    }

}