/*
	header files
*/
#include "../hal/interrupts/interrupts.h"
#include "thread.h"
/*
	some globals
	for the priority
*/
int scheduler_counter = 0;
/*
	PREEMPTIVE schedular
	round robin, AND also with priority.
	Hybrid?
*/
void scheduler_tick(interrupt_frame_t* frame/*the irq will pass this*/) {
    if (!frame) return; // NULL check
	/*
		IMPORTANT counter to keep track of priority
	*/
	scheduler_counter++;
	/*
		Handle the current thread we point'in
		to
	*/
    if (current_thread) {
        current_thread->context = *frame;
        /*
			Handle the states
		*/
        if (current_thread->state == THREAD_RUNNING) {
            current_thread->state = THREAD_READY;
            add_to_ready_queue(current_thread);
        } else if (current_thread->state == THREAD_TERMINATED) {
            current_thread = NULL;
        } else if (current_thread->state == THREAD_BLOCKED) {
            current_thread = NULL;
        } else {
            current_thread->state = THREAD_READY;
            add_to_ready_queue(current_thread);
        }
    }
    /*
		NEXT
	*/
    thread_t* next_thread = NULL;
    if (ready_queue) {
        next_thread = ready_queue;
        ready_queue = ready_queue->next;
        next_thread->next = NULL;
    }
    /*
		Check for READY threads: They have THREAD_READY
	*/
    if (!next_thread) {
        extern thread_t* thread_table[MAX_THREADS];
        for (int i = 1; i < MAX_THREADS; i++) {
			/*
				Find up the next
			*/
            if (thread_table[i] && thread_table[i]->state == THREAD_READY) {
                next_thread = thread_table[i];
                break;
            }
        }
    }
    
    /*
		Also for the threads who are currently being scheduled
	*/
    if (!next_thread) {
        extern thread_t* thread_table[MAX_THREADS];
        for (int i = 1; i < MAX_THREADS; i++) {
			/*
				handle the current
			*/
            if (thread_table[i] && 
                (thread_table[i]->state == THREAD_READY || 
                 thread_table[i]->state == THREAD_RUNNING)) {
                next_thread = thread_table[i];
                break;
            }
        }
    }
    
    /*
		JUST be with the kernel incase nothing to run
	*/
    if (!next_thread) {
        extern thread_t* thread_table[MAX_THREADS];
        if (thread_table[0] && thread_table[0]->state != THREAD_TERMINATED) {
            next_thread = thread_table[0];
        }
    }
    /*
		Incase no thread
	*/
    if (!next_thread) return;
    
	/*
		Also check if the thread terminated DOESNT slip into the
		ready queue
	*/
    if (next_thread->state == THREAD_TERMINATED) {
        remove_from_ready_queue(next_thread);
        return;
    }
	/*
		Frequency check.
		We have priority check,
		This would be a massive BOOST.
		ALSO:

		Here lower number the better
		We also call these skip rates/stride rate
		we may fine tune this later... for now its good
	*/
	int stride = 1;
	/*
		THE ULTRA ones. so we give these a stride rate of 2
	*/
	if (		next_thread->priority == THREAD_PRIORITY_ULTRA)      	stride = 2;
	/*
		The normal ones, like normal apps which are less intensive, so give them of 4
	*/
	else if (	next_thread->priority == THREAD_PRIORITY_NORMAL) 		stride = 4;
	/*
		The lower ones like services but lets give this a 16
	*/
	else if (	next_thread->priority == THREAD_PRIORITY_LOW)    		stride = 16;
	/*
		And the backbenchers, the background and idler apps with stride of 64
	*/
	else if (	next_thread->priority == THREAD_PRIORITY_BACKGROUND) 	stride = 64;
	/*
		Also we will track the cooldown
	*/
	if (next_thread->cooldown > 0) {
	    next_thread->cooldown--;
	    add_to_ready_queue(next_thread);
		/*
			Try again later
		*/
	    return;
	} else {
		/*
			Reset up the cool down
		*/
	    next_thread->cooldown = stride - 1;
	}
	/*
		This would change
	*/
    current_thread = next_thread;
    current_thread->state = THREAD_RUNNING;
    
    /*
		RESTORE the thread context
		also based of the previalage
		(Spelled it wrong again)
    */
    /*
	 	RING3
    */
    if (current_thread->privilege == THREAD_RING3) {
		/*
			Set the TSS
		*/
        extern tss_t tss;
        
        if (!current_thread->kernel_stack) {
            current_thread->state = THREAD_TERMINATED;
            return;
        }
        /*
			Also handle the kernel stack
		*/
        tss.rsp0 = current_thread->kernel_stack + KERNEL_STACK_SIZE - 8;
        
        if (!current_thread->context.rsp || current_thread->context.rsp < 0x400000000000UL) {
            current_thread->state = THREAD_TERMINATED;
            return;
        }
        /*
			Restore
		*/
        frame->rax = current_thread->context.rax;
        frame->rbx = current_thread->context.rbx;
        frame->rcx = current_thread->context.rcx;
        frame->rdx = current_thread->context.rdx;
        frame->rsi = current_thread->context.rsi;
        frame->rdi = current_thread->context.rdi;
        frame->rbp = current_thread->context.rbp;
        frame->r8 = current_thread->context.r8;
        frame->r9 = current_thread->context.r9;
        frame->r10 = current_thread->context.r10;
        frame->r11 = current_thread->context.r11;
        frame->r12 = current_thread->context.r12;
        frame->r13 = current_thread->context.r13;
        frame->r14 = current_thread->context.r14;
        frame->r15 = current_thread->context.r15;
        /*
			The main of the frame
		*/
        frame->rip = current_thread->context.rip;
        frame->cs = USER_CODE_SELECTOR;
        frame->rflags = current_thread->context.rflags | 0x200;
        frame->rsp = current_thread->context.rsp;
        frame->ss = USER_DATA_SELECTOR;
    } else {
		/*
			RING0
			i was suprised too how less ring 0 takes
 		*/
        *frame = current_thread->context;
    }
}
/*
	Some more helpers
*/

void add_to_ready_queue(thread_t* thread) {
    if (!thread || thread->state == THREAD_TERMINATED) return;
    
    /*
		dobbleganger check
	*/
    thread_t* check = ready_queue;
    while (check) {
        if (check == thread) return;
        check = check->next;
    }
    
    thread->next = NULL;
    thread->state = THREAD_READY;
    
    if (!ready_queue) {
        ready_queue = thread;
    } else {
        thread_t* last = ready_queue;
        while (last->next) last = last->next;
        last->next = thread;
    }
}

void remove_from_ready_queue(thread_t* target) {
    if (!target || !ready_queue) return;
    
    if (ready_queue == target) {
        ready_queue = ready_queue->next;
        target->next = NULL;
        return;
    }
    
    thread_t* current = ready_queue;
    while (current && current->next) {
        if (current->next == target) {
            current->next = target->next;
            target->next = NULL;
            return;
        }
        current = current->next;
    }
}
/*
	NOT used. Because it was helpful to have
	during the realtime schedulaing to have the behaviour of
	this shi i built becuase unpredictable asf
*/
#ifdef D_SCHEDULAR
void debug_scheduler_state(void) {
    printf("=== SCHEDULER DEBUG STATE ===\n");
    
    // Current thread info
    if (current_thread) {
        printf("Current Thread: TID=%d PID=%d State=%d\n", 
               current_thread->tid, current_thread->owner_pid, current_thread->state);
    } else {
        printf("Current Thread: NULL\n");
    }
    
    // Ready queue info
    printf("Ready Queue: ");
    thread_t* q = ready_queue;
    int count = 0;
    while (q && count < 10) { // Prevent infinite loop
        printf("TID=%d ", q->tid);
        q = q->next;
        count++;
    }
    if (count == 0) printf("EMPTY");
    if (count >= 10) printf("...(truncated)");
    printf("\n");
    
    // Thread table info
    extern thread_t* thread_table[MAX_THREADS];
    extern uint32_t thread_count;
    printf("Thread Table: %d threads\n", thread_count);
    
    for (int i = 0; i < MAX_THREADS && i < 10; i++) {
        if (thread_table[i]) {
            printf("  [%d] TID=%d PID=%d State=%d\n", 
                   i, thread_table[i]->tid, thread_table[i]->owner_pid, thread_table[i]->state);
        }
    }
    
    printf("=============================\n");
}
#endif