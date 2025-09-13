#ifndef SYSCALL_H
#define SYSCALL_H
/*
	header files
*/
#include <stdint.h>
#include <stddef.h>
#include "../hal/interrupts/interrupts.h"
/*
	max syscalls
	Idk how many to add
*/
#define MAX_SYSCALLS        99999
/*
	Standard syscall(handler) structure
*/
typedef int64_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
/*
	Standard pretty much
*/
typedef struct {
    syscall_handler_t handler;
    const char* name;
    int arg_count;
} syscall_entry_t;
/*
	keep it static
*/
static syscall_entry_t syscall_table[MAX_SYSCALLS];
/*
	MAIN user space syscall handler macro
*/
#define syscall(num, arg1, arg2, arg3, arg4, arg5, arg6) \
    ({ \
        int64_t result; \
        __asm__ volatile( \
            "movq %1, %%rax\n\t"     /* syscall number */ \
            "movq %2, %%rdi\n\t"     /* arg1 */ \
            "movq %3, %%rsi\n\t"     /* arg2 */ \
            "movq %4, %%rdx\n\t"     /* arg3 */ \
            "movq %5, %%r10\n\t"     /* arg4 */ \
            "movq %6, %%r8\n\t"      /* arg5 */ \
            "movq %7, %%r9\n\t"      /* arg6 */ \
            "int $0x80\n\t"          /* syscall interrupt */ \
            "movq %%rax, %0" \
            : "=r" (result) \
            : "r" ((uint64_t)(num)), "r" ((uint64_t)(arg1)), "r" ((uint64_t)(arg2)), \
              "r" ((uint64_t)(arg3)), "r" ((uint64_t)(arg4)), "r" ((uint64_t)(arg5)), \
              "r" ((uint64_t)(arg6)) \
            : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory" \
        ); \
        result; \
    })
/*
	prototypes
*/
void init_syscalls(void);
void syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
#endif
