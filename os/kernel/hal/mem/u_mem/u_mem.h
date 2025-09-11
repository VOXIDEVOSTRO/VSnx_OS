#ifndef U_MEM_H
#define U_MEM_H
/*
	Header files
    USER VIRTUAL MEMORY MANAGER
    Ring 3 accessible memory management
*/
#include "../k_mem/k_mem.h"
#include "../../../utilities/utility.h"
/*
    USER MEMORY LAYOUT
    0x400000000000 - 0x7FFFFFFFFFFF : User heap space (256TB) of virtual space
*/
#define USER_HEAP_START     0x400000000000ULL
#define USER_HEAP_END       0x7FFFFFFFFFFFFFULL
#define USER_HEAP_SIZE      (USER_HEAP_END - USER_HEAP_START)
/*
	structures
*/
typedef struct vmm_block {
    uint64_t addr;
    size_t size;
    int is_free;
    struct vmm_block* next;
} vmm_block_t;

typedef struct {
    vmm_block_t* free_list;
    uint64_t next_addr;
    uint64_t total_allocated;
    uint64_t total_free;
} vmm_info_t;
/*
	Globals
*/
extern vmm_info_t vmm_info;
/*
    VMM prototypes
*/
void vmm_init(void);
void* umalloc(size_t size);
void ufree(void* ptr);
void* urealloc(void* ptr, size_t new_size);
void vmm_print_info(void);
/*
    User Memory API
	vmm_api.c
*/
void* umalloc(size_t size);
void ufree(void* ptr);
void* urealloc(void* ptr, size_t new_size);
void* ucalloc(size_t num, size_t size);
void* umemcpy(void* dest, const void* src, size_t n);
void* umemset(void* ptr, int value, size_t n);
void* umemmove(void* dest, const void* src, size_t n);
int umemcmp(const void* ptr1, const void* ptr2, size_t n);
void* umemchr(const void* ptr, int value, size_t n);
void vmm_init(void);
void vmm_print_info(void);
#endif