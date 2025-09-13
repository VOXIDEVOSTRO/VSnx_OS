/*
	Header files
*/
#include <stdbool.h>
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
// Add owner_pid parameter to existing function
thread_t* thread_create(thread_func_t func, void* arg, thread_privilege_t privilege/*needed for drivers*/, thread_priority_t priority/*unused probably*/, uint32_t owner_pid) {
    if (thread_count >= MAX_THREADS) return NULL;
    /*
		Alloc some mem
	*/
    thread_t* thread = (thread_t*)kmalloc(sizeof(thread_t));
    if (!thread) return NULL;
	/*
		Allocate stack based on privilege
	*/
    if (privilege == THREAD_RING3) {
		/*
			ring 3
		*/
		/*
			For both the kernel and thread
		*/
        thread->stack_base = (uint64_t)umalloc(THREAD_STACK_SIZE);
        thread->kernel_stack = (uint64_t)kmalloc(KERNEL_STACK_SIZE);
        
        if (!thread->stack_base || !thread->kernel_stack) {
            if (thread->stack_base) ufree((void*)thread->stack_base);
            if (thread->kernel_stack) kfree((void*)thread->kernel_stack);
            kfree(thread);
            return NULL;
        }
    } else {
		/*
			ring 0
		*/
        thread->stack_base = (uint64_t)kmalloc(THREAD_STACK_SIZE);
        thread->kernel_stack = 0;
        
        if (!thread->stack_base) {
            kfree(thread);
            return NULL;
        }
    }
    
	/*
		init the thread
	*/

    thread->tid = next_tid++;
    thread->owner_pid = owner_pid;
    thread->state = THREAD_READY;
    thread->privilege = privilege;
    thread->priority = priority;
    thread->stack_size = THREAD_STACK_SIZE;
    thread->next = NULL;
    
	/*
		Init the context
	*/

    thread->context.rsp = thread->stack_base + THREAD_STACK_SIZE - 8;
    thread->context.rip = (uint64_t)func;
    thread->context.rdi = (uint64_t)arg;
    thread->context.rflags = 0x202;
    thread->context.cs = (privilege == THREAD_RING3) ? USER_CODE_SELECTOR : KERNEL_CODE_SELECTOR;
    thread->context.ss = (privilege == THREAD_RING3) ? USER_DATA_SELECTOR : KERNEL_DATA_SELECTOR;
    
    /*
		add to tabler
	*/
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i] == NULL) {
            thread_table[i] = thread;
            break;
        }
    }
    
    thread_count++; // +1 TID
    printf("THREADING: Created TID=%d for PID=%d\n", thread->tid, owner_pid);
    return thread; // TID
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
    if (!current_thread) {
        printf("THREADING: No current thread to exit\n");
        return;
    }
    /*
		In case if a kernel is calling? i have no idea would it?
	*/
    if (current_thread->tid == 0) {
        printf("THREADING: Cannot exit kernel thread\n");
        return;
    }
    
    printf("THREADING: TID=%d exiting\n", current_thread->tid);
    
    thread_t* exiting_thread = current_thread;
    current_thread->state = THREAD_TERMINATED;
	remove_from_ready_queue(current_thread);
    thread_count--;
    /*
		Brush everything up to clean
	*/
    clean_up(exiting_thread);
    /*
		free thread
	*/
    
    printf("THREADING: Thread exit cleanup complete\n");
	// No return but still the binary may have a return statment
}

/*
	Terminate. ez
*/
int thread_terminate(uint32_t tid) {
    /*
        to prevent a M of the kernel . validate
    */
    if (tid == 0) {
        printf("THREADING: Cannot terminate kernel thread (TID=0)\n");
        return -1;
    }
    
    /*
        Find the thread duh
    */
    thread_t* target_thread = NULL;
    
    for (int i = 1; i < MAX_THREADS; i++) {
        if (thread_table[i] && thread_table[i]->tid == tid) {
            target_thread = thread_table[i];
            break;
        }
    }
    
    /*
        Thread not found (probably doesnt exists)
    */
    if (!target_thread) {
        printf("THREADING: TID=%d not found\n", tid);
        return -2;
    }
    
    printf("THREADING: Terminating TID=%d (PID=%d)\n", target_thread->tid, target_thread->owner_pid);
    
    /*
        Nullify if current
    */
    if (target_thread == current_thread) {
        printf("THREADING: Terminating current thread, clearing current_thread\n");
        current_thread->state = THREAD_TERMINATED;
    }
    
    /*
        Clean up all resources of the thread
    */
    clean_up(target_thread);
    
    /*
        Free the thread structure itself
    */
    kfree(target_thread);
    
    printf("THREADING: TID=%d terminated successfully\n", tid);
    return 0;
}
/*
	Small implimnatation for getting the thread id
*/
uint32_t gettid(void) {
    if (current_thread == NULL) {
        return 0;
    }
    return current_thread->tid;
}

uint32_t getpid(void) {
    if (current_thread == NULL) {
        return 0;
    }
    return current_thread->owner_pid;
}

thread_t* thread_get_by_tid(uint32_t tid) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i] && thread_table[i]->tid == tid) {
            return thread_table[i];
        }
    }
	/*
		Thread missing
	*/
    return NULL;
}

/*
    Check if thread exists by TID
*/
bool thread_exists(uint32_t tid) {
    return thread_get_by_tid(tid) != NULL;
}

/*
    Get thread state by TID
*/
thread_state_t thread_get_state(uint32_t tid) {
    thread_t* thread = thread_get_by_tid(tid);
    if (!thread) {
        return -1;
    }
    return thread->state;
}

/*
	Some healperz
*/

void clean_up(thread_t* thread) {
    if (!thread) return;
    
    printf("THREADING: Cleaning up resources for TID=%d\n", thread->tid);

	/*
		MOST important fix:
		REMOVE AND CALL schedular to do a STACK change!
		becuase the thread_exit RETURNS but the issue is
		we cant stop it via a like a HALT loop or something.
		So toavoid page faults. WE dont free the stack until
		the scheduler is at a diffrent stack so in the return
		we resturn to a already active stack and free later.
	*/

	remove_from_ready_queue(thread);
    __asm__ volatile("int $0x20");

    if (thread->privilege == THREAD_RING3) {
        if (thread->stack_base) {
            ufree((void*)thread->stack_base);
            thread->stack_base = 0;
        }
        if (thread->kernel_stack) {
            kfree((void*)thread->kernel_stack);
            thread->kernel_stack = 0;
        }
    } else {
        if (thread->stack_base) {
            kfree((void*)thread->stack_base);
            thread->stack_base = 0;
        }
    }
    
    /*
		Remove from table
		or else confustion
	*/
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i] == thread) {
            thread_table[i] = NULL;
            break;
        }
    }
    
    // duh...
    thread->state = THREAD_TERMINATED;
    
    thread_count--; // -1
} 
