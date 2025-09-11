/*
	Header files
*/
#include "thread.h"
#include "../utilities/utility.h"
#include "../hal/mem/k_mem/k_mem.h"
#include "../hal/mem/u_mem/u_mem.h"
/*
	Globals
	declaring here
*/
thread_t* thread_table[MAX_THREADS];
thread_t* current_thread = NULL;
thread_t* ready_queue = NULL;
uint32_t next_tid = 1;
uint32_t thread_count = 0;
/*
	INIT the threading! or multithreading if you want that way
*/
void threading_init(void) {
    printf("THREADING: Initializing\n");
    
	/*
		validate the table
	*/
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_table[i] = NULL;
    }
    
    /*
		We need the kernel as the master thread
	*/
    thread_t* kernel_thread = (thread_t*)kmalloc(sizeof(thread_t));
    kernel_thread->tid = 0;
    kernel_thread->state = THREAD_RUNNING;
    kernel_thread->privilege = THREAD_RING0;
    kernel_thread->priority = THREAD_PRIORITY_KERNEL;
    kernel_thread->next = NULL;
    
    current_thread = kernel_thread;
    thread_table[0] = kernel_thread;
    thread_count = 1;
    
    printf("THREADING: Ready SET Go!!!!!\n");
}
/*
	Simple thread creation
*/
thread_t* thread_create(thread_func_t func, void* arg, thread_privilege_t privilege/*FOR drivers and stuff we need this and maybe for syscall entry we would make wrapper*/, thread_priority_t priority) {
    if (thread_count >= MAX_THREADS) return NULL;
    
	/*
		Alloc the thread duh..
	*/
    thread_t* thread = (thread_t*)kmalloc(sizeof(thread_t));
    if (!thread) return NULL;
    
    /*
		alloc the stack for both user and kernel
	*/
    if (privilege == THREAD_RING3) {
        thread->stack_base = (uint64_t)umalloc(THREAD_STACK_SIZE);  // User stack (we will change RSP)
        thread->kernel_stack = (uint64_t)kmalloc(KERNEL_STACK_SIZE); // Kernel stack (for interrupts)
        /*
			Also validate the stack allocation
		*/
        if (!thread->stack_base || !thread->kernel_stack) {
            if (thread->stack_base) ufree((void*)thread->stack_base);
            if (thread->kernel_stack) kfree((void*)thread->kernel_stack);
            kfree(thread);
            return NULL;
        }
    } else {
        /*
			RING 0! use the kernel stack
		*/
        thread->stack_base = (uint64_t)kmalloc(THREAD_STACK_SIZE);
        thread->kernel_stack = 0;
        /*
			validate the stack
		*/
        if (!thread->stack_base) {
            kfree(thread); // error
            return NULL;
        }
    }
    
    /*
		INIT the thread values
	*/
    thread->tid = next_tid++;
    thread->state = THREAD_READY;
    thread->privilege = privilege;
    thread->priority = priority;
    thread->stack_size = THREAD_STACK_SIZE;
    thread->next = NULL;
    
    /*
		set the conext up
	*/
    thread->context.rsp = thread->stack_base + THREAD_STACK_SIZE - 8;
    thread->context.rip = (uint64_t)func;
    thread->context.rdi = (uint64_t)arg;
    thread->context.rflags = 0x202;
    thread->context.cs = (privilege == THREAD_RING3) ? USER_CODE_SELECTOR : KERNEL_CODE_SELECTOR; /*based upon the requested ring*/
    thread->context.ss = (privilege == THREAD_RING3) ? USER_DATA_SELECTOR : KERNEL_DATA_SELECTOR;
    
    // Add to table. so we can lookup
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i] == NULL) {
            thread_table[i] = thread;
            break;
        }
    }
    
    thread_count++; // +1
    printf("THREADING: Created TID=%d\n", thread->tid);
    return thread; // tid
}

/*
	VERY simple thread execution
	the rest is handled by the scheduler. just add the thread to the ready queue
*/
int thread_execute(thread_t* thread) {
	/*
		validate the thread first
	*/
    if (!thread || thread->state != THREAD_READY) return -1;
    
    printf("THREADING: Executing TID=%d\n", thread->tid);
    
    // Add to ready queue . simple asf
    if (ready_queue == NULL) {
        ready_queue = thread;
    } else {
        thread_t* last = ready_queue;
        while (last->next) last = last->next;
        last->next = thread;
    }
    
    return 0;
}
/*
	Also very simple just remove from ready queue and free the resources
*/
void thread_exit(void) {
	/*
		Validate and make sure its NOT the kernel OR CHAOS
	*/
    if (!current_thread || current_thread->tid == 0) return;
    
    printf("THREADING: TID=%d exiting\n", current_thread->tid);
    
    /*
		Saving up
	*/
    if (current_thread->privilege == THREAD_RING3) {
        ufree((void*)current_thread->stack_base);
    } else {
        kfree((void*)current_thread->stack_base);
    }
    
    // Remove from table to avoid the schedular intererfering nulled our code
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i] == current_thread) {
            thread_table[i] = NULL;
            break;
        }
    }
    
	/*free the thread itself*/
    kfree(current_thread);
    thread_count--;
    
    // TO BE REMOVED! BECUASE SCHEDULAR DOESNT care it just want to see the queue and THIS may cause problems!
	/*
    if (ready_queue) {
        current_thread = ready_queue;
        ready_queue = ready_queue->next;
        current_thread->next = NULL;
        current_thread->state = THREAD_RUNNING;
        
        // Jump to thread
        __asm__ volatile("mov %0, %%rsp; jmp *%1" 
            : : "m"(current_thread->context.rsp), "m"(current_thread->context.rip));
    }
	*/
}
