#ifndef PIPE_H
#define PIPE_H
/*
	Headerz filez
*/
#include <stdarg.h>
#include <stdint.h>
#include "../../hal/mem/k_mem/k_mem.h" // For some mem
#include "../../utilities/utility.h" // The string functions
/*
	The function ptr struct
*/
typedef void (*pipe_t)(void* data);
/*
	Main pipe entries
*/
typedef struct pipe_entry {
    char name[64/*MAX*/];
    pipe_t func;
    struct pipe_entry* next;
} pipe_entry_t;
/*
	The data we will pass as a struct or array
*/
struct pipe_data {
    uint64_t args[10/*MAX*/];
};
/*
	The pipe list global
*/
extern pipe_entry_t* pipe_list;
/*
	prototypes 
*/
#ifndef USER
void makepipe(const char* name, pipe_t func);
pipe_t getpipe(const char* name);
#endif
#endif
