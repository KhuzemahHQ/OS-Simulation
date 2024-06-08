#include "thread.h"

// mask used to ignore some signals
// we need to ignore the alarm clock signal when we are doing critical stuff
// an empty masks means all signals are enabled
sigset_t signal_mask;

// the timer for the alarm clock
struct itimerval timer;

// stores the pointer to the currently running thread
struct TCB* running = NULL;
int num_threads = 0;

void init_lib() {

}

// returns id of the current thread
int self_id() {
    return running->thread_id;
}

int get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);W

    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

// manages the threads and decides on which thread to run next
void scheduler(int signal_number) {
    printf("scheduler called \n");
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
