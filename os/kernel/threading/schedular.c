/*
	header files
*/
#include "../hal/interrupts/interrupts.h"
#include "thread.h"
/*
	some globals
	for the priority
*/
int scheduler_counter/*Should've name it the priority counter*/ = 0;
/*
	PREEMPTIVE schedular
	round robin, AND also with priority.
	Hybrid?
*/
void scheduler_tick(interrupt_frame_t* frame/*the irq will pass this*/) {
    if (!frame) return; // NULL check
	/*
		Main scheduler counter
		or priority or stride counter
		it has a lot of names
	*/
    scheduler_counter++;
	/*
		handle up the states here
		This is the pre-state check
		to avoid a slip in to the queue
	*/
    if (current_thread) {
		/*

			Save context

		*/
        current_thread->context = *frame;
		/*
			running and ready ones
		*/
        if (current_thread->state == THREAD_RUNNING) {
            current_thread->state = THREAD_READY;
            add_to_ready_queue(current_thread);
		/*
			terminated ones
		*/
        } else if (current_thread->state == THREAD_TERMINATED) {
            remove_from_ready_queue(current_thread);
            current_thread = NULL;
		/*
			Blocked threads such as drivers and stuff
		*/
        } else if (current_thread->state == THREAD_BLOCKED) {
            remove_from_ready_queue(current_thread);
            current_thread = NULL;
		/*
			And the ready ones or the newbie threads
		*/
        } else {
            current_thread->state = THREAD_READY;
            add_to_ready_queue(current_thread);
        }
    }

	/*
		Set or init the variable for
		the next thread for round robin
	*/

    thread_t* next_thread = NULL;

/*
	You can say this as the 
	main fall back for skipping the
	threads in the round robin
*/

select_again:;
	/*
		Find the next thread
	*/
    if (ready_queue) {
        next_thread = ready_queue;
        ready_queue = ready_queue->next;
        next_thread->next = NULL;
    }
	/*
		If no next thread
		get the next thread. duh..
	*/
    if (!next_thread) {
        extern thread_t* thread_table[MAX_THREADS];
        for (int i = 1; i < MAX_THREADS; i++) {
			/*
				Loop to get and set as ready
			*/
            if (thread_table[i] && thread_table[i]->state == THREAD_READY) {
                next_thread = thread_table[i];
                break;
            }
        }
    }
	/*
		If still non found
		it means no thread yet
		So if any stray set all as terminated
		and focus on the kernel
	*/
    if (!next_thread) {
        extern thread_t* thread_table[MAX_THREADS];
        if (thread_table[0] && thread_table[0]->state != THREAD_TERMINATED) {
            next_thread = thread_table[0];
        }
    }
	/*
		If even the kernel doesnt exists???
		Just return and fault up
	*/
    if (!next_thread) return;
	/*
		PICK up the non runnables
		such as the terminated ones
		and blocked, incase from the pre-
		check they slipped in
	*/
    if (next_thread->state == THREAD_BLOCKED || next_thread->state == THREAD_TERMINATED) {
		/*
			Remove
		*/
        remove_from_ready_queue(next_thread);
        next_thread = NULL/*will be set later again*/;
		/*
			Find again
		*/
        goto select_again;
    }
	/*
		PRIORITY handler:
		it works via adjusting the scheduling frequency
		which is extremely simple and this freq is denoted
		via something called a "stride" OR "priority freq".
		and each thread has cooldown counter which tracks the
		remaining freq
	*/
    int stride = 1;
	/*
		Handle up the ULTRA ones
		With a stride of 2
		and used by the MAIN running active applications
		such as games, GUI tools or lots of other apps
		which are currently active
	*/
    if (next_thread->priority == THREAD_PRIORITY_ULTRA)              stride = 2;
	/*
		Normal handler:
		More less GUI intensive but still in active status
		with a stride of 4
		like IDEs, text editors, or lightweight games or even simple GUI
		tools
	*/
    else if (next_thread->priority == THREAD_PRIORITY_NORMAL)        stride = 4;
	/*
		Lower priority ones or also called the service priority,
		Stride of 16, these are mostly less intensive and passive applications
		or executables such as cmdlets, background services and a temp thread
	*/
    else if (next_thread->priority == THREAD_PRIORITY_LOW)           stride = 16;
	/*
		Background ones or also called the idle priority,
		Stride of 64, these are the COMPLETE passive applications or
		background services, which only handle normal stuff.
	*/
    else if (next_thread->priority == THREAD_PRIORITY_BACKGROUND)    stride = 64;
	/*
		Handle the cooldown of each priority
		this took a while to troubleshoot
	*/
    if (next_thread->cooldown > 0) {
		/*
			-1 the counter if already in the counter
		*/
        next_thread->cooldown--;
        add_to_ready_queue(next_thread);
        next_thread = NULL;
		/*
			Go back
		*/
        goto select_again;
    } else {
		/*
			If done reset the -1 the stride
		*/
        next_thread->cooldown = stride - 1;
    }
	/*
		get the next running or being schduled
	*/
    current_thread = next_thread;
    current_thread->state = THREAD_RUNNING;
	/*
	
		MAIN restoration
	
	*/
	/*
		RING3
	*/
    if (current_thread->privilege == THREAD_RING3) {
        extern tss_t tss;
		/*
			handle invalid thread contexts
		*/
        if (!current_thread->kernel_stack) {
            current_thread->state = THREAD_TERMINATED;
			/*
				Go back
			*/
            goto select_again;
        }
		/*
			TSS checkup and set
		*/
        tss.rsp0 = current_thread->kernel_stack + KERNEL_STACK_SIZE - 8;
		/*
			CHECK and validate the rsp
		*/
        if (!current_thread->context.rsp /*This is not needed|| current_thread->context.rsp < 0x400000000000UL*/) {
            current_thread->state = THREAD_TERMINATED;
			/*
				Retry
			*/
            goto select_again;
        }
		/*
			RESTORE the general purpose
			registers from the frame
		*/
        frame->rax = current_thread->context.rax;
        frame->rbx = current_thread->context.rbx;
        frame->rcx = current_thread->context.rcx;
        frame->rdx = current_thread->context.rdx;
        frame->rsi = current_thread->context.rsi;
        frame->rdi = current_thread->context.rdi;
        frame->rbp = current_thread->context.rbp;
        frame->r8  = current_thread->context.r8;
        frame->r9  = current_thread->context.r9;
        frame->r10 = current_thread->context.r10;
        frame->r11 = current_thread->context.r11;
        frame->r12 = current_thread->context.r12;
        frame->r13 = current_thread->context.r13;
        frame->r14 = current_thread->context.r14;
        frame->r15 = current_thread->context.r15;
		/*
			Main of the Frame
		*/
        frame->rip    = current_thread->context.rip;
        frame->cs     = USER_CODE_SELECTOR/*1B*/;
        frame->rflags = current_thread->context.rflags | /*Enable interrupts*/0x200;
        frame->rsp    = current_thread->context.rsp;
        frame->ss     = USER_DATA_SELECTOR/*23*/;
    } else {
		/*

			RING0

		*/
		/*
			Restore the 
			registers of the ring0
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