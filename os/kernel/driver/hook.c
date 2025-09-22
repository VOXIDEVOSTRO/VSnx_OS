/*
	Includes or just headers
*/
#include "hook.h"
/*
	The main dispatch table or list
*/
hook_entry_t* hook_list = NULL;
/*
	main registering function and suprisingly simple
*/
void register_hook(const char* name, hook_func_t func) {
    hook_entry_t* current = hook_list;
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
    hook_entry_t* entry = (hook_entry_t*)/*PMM*/kmalloc(sizeof(hook_entry_t));
	/*
		validate
	*/
    if (!entry) return;
    strncpy(entry->name, name, 63);
	/*
		Init the hook
	*/
    entry->name[63] = '\0'; // Null terminate DUH
    entry->func = func;
    entry->next = hook_list;
    hook_list = entry;
}
/*
	The main caller has a very simple job
*/
int64_t call_hook(const char* name, void* data) {
    hook_entry_t* current = hook_list;
    while (current) {
		/*
			Simple job just get the function pointer
			and call it. Much better then before
		*/
        if (strcmp(current->name, name) == 0) {
            return current->func(data);
        }
        current = current->next;
    }
	/*
		Nothing
	*/
    return -1;
}
/*
	Just get the pointer
*/
hook_func_t get_hook(const char* name) {
    hook_entry_t* current = hook_list;
	/*
		Simple getting for hook
		we return the pointer
		and great for direct acsess
	*/
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->func;
        }
        current = current->next;
    }
    return NULL;
}
