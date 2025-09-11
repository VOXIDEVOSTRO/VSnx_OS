#ifndef K_MEM_H
#define K_MEM_H
/*
	header files
*/
#include <stdint.h>
#include <stddef.h>
#include "../../multiboot2/multiboot2.h"
/*
	pages
*/
// Physical memory constants
#define PAGE_SIZE           4096 // 4KB
#define PAGE_ALIGN(addr)    ((addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(addr) (addr & ~(PAGE_SIZE - 1))
/*
	memory bit map
*/
// Memory bitmap
#define BITMAP_USED         1
#define BITMAP_FREE         0
/*
	PMM info struct
*/
typedef struct {
    uint64_t memory_start;
    uint64_t memory_end;
    uint64_t total_pages;
    uint64_t free_pages;
    uint64_t used_pages;
    uint8_t* bitmap;
    uint64_t bitmap_size;
    uint64_t next_alloc_hint;  // Speed up allocation
} pmm_info_t;

// Page mapping on-demand
typedef struct {
    uint64_t virt_addr;
    uint64_t phys_addr;
    uint32_t flags;
} page_mapping_t;
/*
	page flags
*/
// Page table entry flags
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_2MB   0x080
#define PAGE_GLOBAL     0x100
#define PAGE_NX         0x8000000000000000ULL
/*
	page tables
*/
// Page table structures
typedef struct {
    uint64_t entries[512];
} __attribute__((aligned(4096))) page_table_t;
typedef struct {
    page_table_t* pml4;
    uint64_t next_table_addr;
    uint32_t total_tables;
    uint32_t max_tables;
} vsnx_page_manager_t;
/*
	globals
*/
extern pmm_info_t pmm_info;
extern uint64_t next_page_table_addr;
extern vsnx_page_manager_t page_mgr;
/*
	prototypes
*/
void pmm_init(void);
void pmm_map_entire_memory(void);
int pmm_test_memory_regions(void);
uint64_t find_contiguous_pages(size_t pages_needed);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t page_addr);
void pmm_mark_page_used(uint64_t page_addr);
void pmm_mark_page_free(uint64_t page_addr);
uint64_t pmm_get_total_pages(void);
int is_address_available(uint64_t addr);
uint64_t pmm_get_free_pages(void);
void pmm_print_info(void);
/*
	paging.c
*/
int paging_map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags);
void paging_unmap_page(uint64_t vaddr);
uint64_t paging_get_physical(uint64_t vaddr);
void paging_print_info(void);
/*
    Kernel Memory API
	pmm_api.c
*/
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void* kcalloc(size_t num, size_t size);
void* kmemcpy(void* dest, const void* src, size_t n);
void* kmemset(void* ptr, int value, size_t n);
void* kmemmove(void* dest, const void* src, size_t n);
int kmemcmp(const void* ptr1, const void* ptr2, size_t n);
void* kmemchr(const void* ptr, int value, size_t n);
#endif