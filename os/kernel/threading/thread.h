#ifndef THREAD_H
#define THREAD_H
/*
	Header files
*/
#include <stdint.h>
#include "../hal/interrupts/idt/idt.h"
/*
	threads structures
*/
// Thread states
typedef enum {
    THREAD_READY = 0,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

// Thread privilege levels
typedef enum {
    THREAD_RING0 = 0,    // Kernel mode
    THREAD_RING3 = 3     // User mode
} thread_privilege_t;

// Thread priority levels
typedef enum {
    THREAD_PRIORITY_LOW = 0,
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_HIGH,
    THREAD_PRIORITY_KERNEL
} thread_priority_t;
/*
	Thread control block
*/
typedef struct thread {
    uint32_t tid;                    // Thread ID
    thread_state_t state;            // Current state
    thread_privilege_t privilege;    // Ring level (0 or 3)
    thread_priority_t priority;      // Thread priority
    interrupt_frame_t context;       // CPU context (IDT frame)
    uint64_t stack_base;             // Stack base address
    uint64_t stack_size;             // Stack size
    uint64_t kernel_stack;           // Kernel stack for ring transitions
    uint32_t time_slice;             // Time quantum
    uint32_t time_used;              // Time used in current slice
    struct thread* next;             // Next thread in scheduler queue
    void* user_data;                 // User-defined data
} thread_t;

// Thread function pointer
typedef void (*thread_func_t)(void* arg);

// Thread management
#define MAX_THREADS 256
#define THREAD_STACK_SIZE 0x4000  // 16KB stack per thread /*That too big tbh*/
#define KERNEL_STACK_SIZE 0x2000  // 8KB kernel stack
/*
	Globals
*/
// Global threading states (FREE DOM!!!!!)
extern thread_t* thread_table[MAX_THREADS];
extern thread_t* current_thread;
extern thread_t* ready_queue;
extern uint32_t next_tid;
extern uint32_t thread_count;
/*
	Prototypes
*/
void threading_init(void);
thread_t* thread_create(thread_func_t func, void* arg, thread_privilege_t privilege, thread_priority_t priority);
int thread_execute(thread_t* thread);
void thread_exit(void);
/*
	Schedular
*/
void scheduler_tick(interrupt_frame_t* frame);
#endif
