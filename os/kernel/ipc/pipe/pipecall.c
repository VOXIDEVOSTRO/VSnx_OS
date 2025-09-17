/*
	Includes or just headers
*/
#include "pipe.h"
/*
	The main dispatch table or list
*/
pipe_entry_t* pipe_list = NULL;
/*
	main registering function and suprisingly simple
*/
void makepipe(const char* name, pipe_t func) {
    pipe_entry_t* current = pipe_list;
    while (current) {
		/*
			compare the entry name
		*/
        if (strcmp(current->name, name) == 0) {
            current->func = func;
            return;
        }
        current = current->next;
    }
	/*
		Alloc the entry some memory
	*/
    pipe_entry_t* entry = (pipe_entry_t*)kmalloc(sizeof(pipe_entry_t));
    if (!entry) return;
    strncpy(entry->name, name, 63);
	/*
		Init the pipe
	*/
    entry->name[63] = '\0'; // Null terminate DUH
    entry->func = func;
    entry->next = pipe_list;
    pipe_list = entry;
}
pipe_t getpipe(const char* name) {
    pipe_entry_t* current = pipe_list;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->func;
        }
        current = current->next;
    }
    return NULL;
}
