#ifndef HOOKREG_H
#define HOOKREG_H
/*
	header files
*/
#include "hookcall.h"
#include "../hook.h"
/*
	Boring registeration or hooks
*/
static inline int reg_hook(const char* name, hook_func_t func) {
	return hookcall(1/*register hook*/, (uint64_t)name/*The name*/, (uint64_t)func/*The pointer the driver will pass*/, /*rest are NULL just ignore*/0, 0, 0, 0);
}
static inline int call_hookling(const char* name, void* data) {
    return hookcall(2 /* call_hook this will call */, (uint64_t)name /* hook name */, (uint64_t)data /* hook data */, 0, 0, 0, 0);
}
static inline hook_func_t get_hookling(const char* name) {
    return (hook_func_t)hookcall(3 /* get_hook this will get */, (uint64_t)name /* hook name */, 0, 0, 0, 0, 0);
}
/*
	Some mapping stuff
	just in case
*/
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_2MB   0x080
#define PAGE_GLOBAL     0x100
#define PAGE_NX         0x8000000000000000ULL

static inline int map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags) {
    return hookcall(7, vaddr, paddr, flags, 0, 0, 0);
}

static inline void unmap_page(uint64_t vaddr) {
    hookcall(6, vaddr, 0, 0, 0, 0, 0);
}

static inline uint64_t get_physical(uint64_t vaddr) {
    uint64_t out_phys = 0;
    hookcall(5, vaddr, (uint64_t)&out_phys, 0, 0, 0, 0);
    return out_phys;
}
#endif
