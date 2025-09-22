#ifndef HOOK_H
#define HOOK_H
/*
	Headerz filez
*/
#include <stdarg.h>
#include <stdint.h>
#include "../hal/mem/k_mem/k_mem.h" // For some mem
#include "../utilities/utility.h" // The string functions
/*
	The function ptr struct
*/
typedef int64_t (*hook_func_t)(void* data);
/*
	Main hook entries
*/
typedef struct hook_entry {
    char name[64/*MAX*/];
    hook_func_t func;
    struct hook_entry* next;
} hook_entry_t;
/*
	The data we will pass as a struct or array
*/
struct hook_data {
    uint64_t args[10]; // Extended to 10 args because of complex functions too
};
/*
	The hook list global
*/
extern hook_entry_t* hook_list;
/*
	prototypes
*/
void register_hook(const char* name, hook_func_t func);
int64_t call_hook(const char* name, void* data);
hook_func_t get_hook(const char* name);
/*
	Some useful macros
*/
#define HOOK_CALL(name, data) call_hook(name, (void*)(data))
#endif
