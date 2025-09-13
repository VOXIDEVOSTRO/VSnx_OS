/*
	HEADERZ
*/
#include "process.h"
#include "../utilities/utility.h"
#include "../hal/mem/k_mem/k_mem.h"
#include "elf.h"
/*
	Declaring the globals here
*/
process_t* process_table[MAX_PROCESSES];
uint32_t next_pid = 1;
/*
	No spawn egg. LOL i mean spawn the process
*/
process_t* spawn_process(const char* filename, thread_privilege_t privilege) {
    printf("PROCESS: Spawning %s (Ring %d)\n", filename, privilege);
	/*
		First ELF!
	*/
    uint64_t entry_point = elf_parse(filename);
    if (entry_point == 0) {
        printf("PROCESS: Failed to load ELF %s\n", filename);
        return NULL;
    }
	/*
		we need a process structure
	*/
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        printf("PROCESS: Failed to allocate process structure\n");
        return NULL;
    }
	/*
		init the structure
	*/
    proc->pid = next_pid++;
    proc->entry_point = entry_point;
    proc->privilege = privilege;
    proc->next = NULL;
    strncpy(proc->name, filename, 63);
    proc->name[63] = '\0';
    /*
		AND finally the thread
	*/
    proc->main_thread = thread_create((thread_func_t)entry_point, NULL, 
                                 privilege, THREAD_PRIORITY_NORMAL, 
                                 proc->pid);
    /*
		validate the thread
	*/
    if (!proc->main_thread) {
        printf("PROCESS: Failed to create main thread\n");
        kfree(proc);
        return NULL;
    }
    /*
		Add to process table
		just to keep track of
	*/
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            process_table[i] = proc;
            break;
        }
    }
    /*
		print out your PID and TID and entry
	*/
    printf("PROCESS: Created PID=%d, TID=%d, Entry=0x%lx\n", 
           proc->pid, proc->main_thread->tid, entry_point);
    
    return proc;
}
/*
	execute process
	just calls da thread_executo functiono (*some classy french music starts playing*)
*/
int execute_process(process_t* proc) {
    if (!proc || !proc->main_thread) {
        printf("PROCESS: Invalid process for execution\n");
        return -1;
    }
    
    printf("PROCESS: Executing PID=%d\n", proc->pid);
    return thread_execute(proc->main_thread);
}
/*
	SLUATHER THE PROC... Okay i feel sad about this
*/
int kill_process(uint32_t pid) {
    printf("PROCESS: Killing PID=%d and all its threads\n", pid);
    /*
		Now get the threads owned by the process
	*/
    int threads_killed = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i] && thread_table[i]->owner_pid == pid) {
            thread_t* thread = thread_table[i];
            
            printf("PROCESS: Killing thread TID=%d (owned by PID=%d)\n", 
                   thread->tid, pid);
            
			/*
				Free up some space
			*/
            if (thread->privilege == THREAD_RING3) {
                if (thread->stack_base) ufree((void*)thread->stack_base);
                if (thread->kernel_stack) kfree((void*)thread->kernel_stack);
            } else {
                if (thread->stack_base) kfree((void*)thread->stack_base);
            }
            
            /*
			 	Final blowdown: K.O!
			*/
            thread_table[i] = NULL;
            kfree(thread);
            thread_count--;
            threads_killed++;
        }
    }
    
    /*
		Kill this too
	*/
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] && process_table[i]->pid == pid) {
            process_table[i] = NULL;
            kfree(process_table[i]);
            break;
        }
    }
    // done!
    printf("PROCESS: PID=%d terminated (%d threads killed)\n", pid, threads_killed);
    return 0;
}
