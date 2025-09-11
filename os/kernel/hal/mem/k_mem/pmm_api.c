#include "k_mem.h"
#include "../../../utilities/utility.h"

/*
    KERNEL MEMORY API
	or pmm api as matters how you call it
*/

/*
    kmalloc - allocate memory
*/
void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    if (pages_needed == 1) {
        uint64_t page = pmm_alloc_page();
        return page ? (void*)page : NULL;
    }
    
    // Multi-page allocation
    for (uint64_t i = 0; i <= pmm_info.total_pages - pages_needed; i++) {
        int found = 1;
        
        for (size_t j = 0; j < pages_needed; j++) {
            uint64_t page_addr = pmm_info.memory_start + ((i + j) * PAGE_SIZE);
            
            if (!is_address_available(page_addr)) {
                found = 0;
                break;
            }
            
            uint64_t byte_index = (i + j) / 8;
            uint64_t bit_index = (i + j) % 8;
            
            if (pmm_info.bitmap[byte_index] & (1 << bit_index)) {
                found = 0;
                break;
            }
        }
        
        if (found) {
            for (size_t j = 0; j < pages_needed; j++) {
                uint64_t byte_index = (i + j) / 8;
                uint64_t bit_index = (i + j) % 8;
                pmm_info.bitmap[byte_index] |= (1 << bit_index);
                pmm_info.used_pages++;
            }
            
            return (void*)(pmm_info.memory_start + (i * PAGE_SIZE));
        }
    }
    
    return NULL;
}

/*
    kfree - free memory
*/
void kfree(void* ptr) {
    if (!ptr) return;
    pmm_free_page(PAGE_ALIGN_DOWN((uint64_t)ptr));
}

/*
    krealloc - reallocate memory
*/
void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    void* new_ptr = kmalloc(new_size);
    if (new_ptr) {
        kmemcpy(new_ptr, ptr, PAGE_SIZE < new_size ? PAGE_SIZE : new_size);
        kfree(ptr);
    }
    return new_ptr;
}

/*
    kcalloc - allocate and zero memory
*/
void* kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    if (total_size / num != size) return NULL; // Overflow check
    
    void* ptr = kmalloc(total_size);
    if (ptr) {
        kmemset(ptr, 0, total_size);
    }
    return ptr;
}

/*
    kmemcpy - copy memory
*/
void* kmemcpy(void* dest, const void* src, size_t n) {
    if (!dest || !src) return dest;
    
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/*
    kmemset - set memory to value
*/
void* kmemset(void* ptr, int value, size_t n) {
    if (!ptr) return ptr;
    
    unsigned char* p = (unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = val;
    }
    
    return ptr;
}

/*
    kmemmove - move memory (handles overlapping regions)
*/
void* kmemmove(void* dest, const void* src, size_t n) {
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
    kmemcmp - compare memory
*/
int kmemcmp(const void* ptr1, const void* ptr2, size_t n) {
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
    kmemchr - find byte in memory
*/
void* kmemchr(const void* ptr, int value, size_t n) {
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
