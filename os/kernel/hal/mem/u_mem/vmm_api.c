#include "u_mem.h"
#include "../../../utilities/utility.h"

/*
    USER MEMORY API
	or vmm?
*/
/*
    User malloc - ring 3 accessible
*/
void* umalloc(size_t size) {
    if (size == 0) return NULL;
    /*
		Mak sure of the alignement. align to page boundries
	*/
    size = PAGE_ALIGN(size);
    /*
		Check the free pages
	*/
	vmm_block_t* current = vmm_info.free_list;
	while (current) {
	    if (current->is_free && current->size >= size) {
	        current->is_free = 0;
	        vmm_info.total_allocated += current->size;
	        vmm_info.total_free -= current->size;
			/*
				IMPORTANT we must remap if reusing the page
			*/
	        size_t pages_needed = current->size / PAGE_SIZE;
	        for (size_t i = 0; i < pages_needed; i++) {
	            uint64_t virt_page = current->addr + (i * PAGE_SIZE);
	            uint64_t phys_page = pmm_alloc_page();
	            if (!phys_page || !paging_map_page(virt_page, phys_page, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER)) {
	                // Handle error
	                return NULL; // NULLERRO
	            }
	        }
		
	        return (void*)current->addr;
	    }
	    current = current->next;
	}
    
    /*
		Imppossible to reach here but just incase to avoid pagefaults
	*/
    if (vmm_info.next_addr + size > USER_HEAP_END) {
        return NULL; // Out of virtual space
    }
    
    /*
		Pmms role here to get the physical page and map it virtually in te VMM
	*/
    size_t pages_needed = size / PAGE_SIZE;
    uint64_t virt_start = vmm_info.next_addr;
    
    for (size_t i = 0; i < pages_needed; i++) {
        uint64_t virt_page = virt_start + (i * PAGE_SIZE);
        uint64_t phys_page = pmm_alloc_page();
        
        if (!phys_page) {
            // Cleanup partial allocation
            for (size_t j = 0; j < i; j++) {
                uint64_t cleanup_virt = virt_start + (j * PAGE_SIZE);
                uint64_t cleanup_phys = paging_get_physical(cleanup_virt);
                paging_unmap_page(cleanup_virt);
                pmm_free_page(cleanup_phys);
            }
            return NULL;
        }
        
        if (!paging_map_page(virt_page, phys_page, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER)) {
            pmm_free_page(phys_page);
            return NULL;
        }
    }
    
    /*
		We need a block descriptor too
	*/
    vmm_block_t* block = (vmm_block_t*)kmalloc(sizeof(vmm_block_t));
    if (!block) {
        // Cleanup allocation
        for (size_t i = 0; i < pages_needed; i++) {
            uint64_t cleanup_virt = virt_start + (i * PAGE_SIZE);
            uint64_t cleanup_phys = paging_get_physical(cleanup_virt);
            paging_unmap_page(cleanup_virt);
            pmm_free_page(cleanup_phys);
        }
        return NULL;
    }

	/*
		initilize the allocation of the VMM
	*/
    
    block->addr = virt_start;
    block->size = size;
    block->is_free = 0;
    block->next = vmm_info.free_list;
    vmm_info.free_list = block;
    
    vmm_info.next_addr += size;
    vmm_info.total_allocated += size;
    
    return (void*)virt_start;
}

/*
    User free - ring 3 accessible
*/
void ufree(void* ptr) {
    if (!ptr) return;
    
    uint64_t addr = (uint64_t)ptr;
    vmm_block_t* current = vmm_info.free_list;
    
    while (current) {
        if (current->addr == addr && !current->is_free) {
            current->is_free = 1;
            vmm_info.total_allocated -= current->size;
            vmm_info.total_free += current->size;
            
            // Unmap and free physical pages
            size_t pages = current->size / PAGE_SIZE;
            for (size_t i = 0; i < pages; i++) {
                uint64_t virt_page = current->addr + (i * PAGE_SIZE);
                uint64_t phys_page = paging_get_physical(virt_page);
                if (phys_page) {
                    paging_unmap_page(virt_page);
                    pmm_free_page(phys_page);
                }
            }
            break;
        }
        current = current->next;
    }
}
/*
    User realloc - ring 3 accessible
	simple just create new allocation. copy data free old allocation
*/
void* urealloc(void* ptr, size_t new_size) {
	/*
		Tons of validation here
	*/
    if (!ptr) return umalloc(new_size);
    if (new_size == 0) {
        ufree(ptr);
        return NULL;
    }
    
    // Find current block to put here
    uint64_t addr = (uint64_t)ptr;
    vmm_block_t* current = vmm_info.free_list;
    
	/*
		get the block
	*/
    while (current) {
        if (current->addr == addr && !current->is_free) {
            size_t old_size = current->size;
            new_size = PAGE_ALIGN(new_size);
            
			/*
				TO BE CHANGED we would change this to be more flexible and make it so if we want a smaller alloc!
			*/
            if (new_size <= old_size) {
                return ptr; // Same or smaller Doesnt matter tbh
            }
            
            // Need larger block
            void* new_ptr = umalloc(new_size);
            if (new_ptr) {
                // Copy old data just as it is
                for (size_t i = 0; i < old_size; i++) {
                    ((char*)new_ptr)[i] = ((char*)ptr)[i];
                }
                ufree(ptr);
            }
            return new_ptr;
        }
        current = current->next;
    }
    
    return NULL;
}
/*
    umemcpy - copy memory
	Simple copy memory from src to dest
*/
void* umemcpy(void* dest, const void* src, size_t n) {
    if (!dest || !src) return dest;
    
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/*
    umemset - set memory to value
*/
void* umemset(void* ptr, int value, size_t n) {
    if (!ptr) return ptr;
    
    unsigned char* p = (unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = val;
    }
    
    return ptr;
}

/*
    umemmove - move memory 
	(handles overlapping regions)
*/
void* umemmove(void* dest, const void* src, size_t n) {
    if (!dest || !src) return dest;
    
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        for (size_t i = n; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
    
    return dest;
}

/*
    umemcmp - compare memory
*/
int umemcmp(const void* ptr1, const void* ptr2, size_t n) {
    if (!ptr1 || !ptr2) return 0;
    
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) return -1;
        if (p1[i] > p2[i]) return 1;
    }
    
    return 0;
}

/*
    umemchr - find byte in memory
*/
void* umemchr(const void* ptr, int value, size_t n) {
    if (!ptr) return NULL;
    
    const unsigned char* p = (const unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    
    for (size_t i = 0; i < n; i++) {
        if (p[i] == val) {
            return (void*)(p + i);
        }
    }
    
    return NULL;
}

/*
    ucalloc - allocate and zero memory
*/
void* ucalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    if (total_size / num != size) return NULL; // Overflow check
    
    void* ptr = umalloc(total_size);
    if (ptr) {
        umemset(ptr, 0, total_size);
    }
    return ptr;
}
