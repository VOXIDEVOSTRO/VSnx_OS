/*
	Header files
*/
#include <stdbool.h>
#include "hookcall.h"
#include "../hook.h"
#include "../../threading/thread.h"
#include "../../elf/process.h"
#include "../../fat32/gristle.h" // For fat32
#include "../../hal/mem/u_mem/u_mem.h" // For memory
/*
	Some hooks
*/

int64_t register_hook_handler(uint64_t name_ptr, uint64_t func_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    const char* name = (const char*)name_ptr;
    hook_func_t func = (hook_func_t)func_ptr;
    register_hook(name, func);
    return 0;
}
int64_t call_hook_handler(uint64_t name_ptr, uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    const char* name = (const char*)name_ptr;
    void* data = (void*)data_ptr;
    return call_hook(name, data);
}
int64_t get_hook_handler(uint64_t name_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    const char* name = (const char*)name_ptr;
    hook_func_t func = get_hook(name);
    return (int64_t)func;
}

/*






	Pagaing Handlers





*/

int64_t paging_map_page_handler(uint64_t vaddr, uint64_t paddr, uint64_t flags, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    return paging_map_page(vaddr, paddr, (uint32_t)flags);
}

int64_t paging_unmap_page_handler(uint64_t vaddr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    paging_unmap_page(vaddr);
    return 0;
}

int64_t paging_get_physical_handler(uint64_t vaddr, uint64_t out_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    *(uint64_t*)out_ptr = paging_get_physical(vaddr);
    return 0;
}

int64_t paging_print_info_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    paging_print_info();
    return 0;
}
