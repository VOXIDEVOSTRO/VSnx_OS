#ifndef HOOKSET_H
#define HOOKSET_H
/*
	header files
*/
#include "hookcall.h"
/*
	Prototypes
*/
int64_t register_hook_handler(uint64_t type, uint64_t func, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
int64_t get_hook_handler(uint64_t name_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t call_hook_handler(uint64_t name_ptr, uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

int64_t paging_map_page_handler(uint64_t vaddr, uint64_t paddr, uint64_t flags, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_unmap_page_handler(uint64_t vaddr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_get_physical_handler(uint64_t vaddr, uint64_t out_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_print_info_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
#endif
