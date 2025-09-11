/*
	Header files
*/
#include "u_mem.h"
/*
	Declare the variables here
*/

// Global VMM state
vmm_info_t vmm_info = {0};

/*
    Initialize VMM
*/
void vmm_init(void) {
    printf("VMM: User Virtual Memory Manager\n");
    
	/*
		init the stuff
	*/
    vmm_info.next_addr = USER_HEAP_START;
    vmm_info.free_list = NULL;
    vmm_info.total_allocated = 0;
    vmm_info.total_free = 0;
    
    printf("VMM: User heap 0x%lx - 0x%lx\n", USER_HEAP_START, USER_HEAP_END);
    printf("VMM: Ready for ring 3 access\n");
}

/*
    VMM statistics
	TO BE REMOVED but nice to have
*/
void vmm_print_info(void) {
    printf("=== VMM User Memory Info ===\n");
    printf("Heap range: 0x%lx - 0x%lx\n", USER_HEAP_START, USER_HEAP_END);
    printf("Next allocation: 0x%lx\n", vmm_info.next_addr);
    printf("Total allocated: %lu KB\n", vmm_info.total_allocated / 1024);
    printf("Total free: %lu KB\n", vmm_info.total_free / 1024);
    /*
		simple counter
	*/
    int total_blocks = 0, free_blocks = 0;
    vmm_block_t* current = vmm_info.free_list;
    while (current) {
        total_blocks++;
        if (current->is_free) free_blocks++;
        current = current->next;
    }
    printf("Memory blocks: %d total, %d free\n", total_blocks, free_blocks);
}
