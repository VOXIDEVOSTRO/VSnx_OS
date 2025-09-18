#ifndef PROCESS_H
#define PROCESS_H
/*
	header files
*/
#include <stdint.h>
#include "../threading/thread.h" // sponsored by threading LOL
/*
	Process table/struct. matters on how you say
*/
typedef struct process {
    uint32_t pid;                    // Process ID
    uint64_t entry_point;            // ELF entry point
    thread_t* main_thread;           // Main thread of process
    thread_privilege_t privilege;    // Ring 0 or Ring 3
    struct process* next;            // Process list
    char name[64];                   // Process name
} process_t;
/*
	Some constants and vars
*/
#define MAX_PROCESSES 256 // Could be unlimited too. memory is the limit
extern process_t* process_table[MAX_PROCESSES];
extern uint32_t next_pid;
/*
	prottypes
*/
process_t* spawn_process(const char* filename, thread_privilege_t privilege, thread_priority_t priority);
int kill_process(uint32_t pid);
int execute_process(process_t* proc);
#endif
