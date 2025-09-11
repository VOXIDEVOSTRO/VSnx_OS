/*
	header files
*/
#include "k_mem.h"
#include "../../../utilities/utility.h"// for our own printf
#include "../../serial_com/serial.h"
/*
	Declaring globals
*/
pmm_info_t pmm_info = {0};
/*
    Finds the page
*/
uint64_t find_contiguous_pages(size_t pages_needed) {
    for (uint64_t i = 0; i <= pmm_info.total_pages - pages_needed; i++) {
        int found = 1;
        
        /*
			keeping sure if the pgaes needed are free
		*/
        for (size_t j = 0; j < pages_needed; j++) {
            uint64_t byte_index = (i + j) / 8;
            uint64_t bit_index = (i + j) % 8;
            
            if (pmm_info.bitmap[byte_index] & (1 << bit_index)) {
                found = 0;
                break;
            }
        }
        
		/*
			mark all of them as free
		*/

        if (found) {
            for (size_t j = 0; j < pages_needed; j++) {
                uint64_t page_addr = pmm_info.memory_start + ((i + j) * PAGE_SIZE);
                pmm_mark_page_used(page_addr);
                pmm_info.free_pages--;
                pmm_info.used_pages++;
            }
            
            return pmm_info.memory_start + (i * PAGE_SIZE);
        }
    }
    
    return 0;  // No contiguous space found ERRo! ON THE FACE
}
/*
	Check if that even exists
*/
int is_address_available(uint64_t addr) {
    vsnx_memory_map_t* mmap = get_vsnx_memory_map();
    
	/*
		Using memory map
	*/
    for (uint32_t i = 0; i < mmap->region_count; i++) {
        if (mmap->regions[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start = mmap->regions[i].base_addr;
            uint64_t end = start + mmap->regions[i].length;
            
            if (addr >= start && addr < end && addr >= 0x400000) {
                return 1;
            }
        }
    }
    return 0;  // this adresses isnt there to give service
}
/*
	INIT THE Physical memory manager
*/
void pmm_init(void) {
    printf("PMM: on-demand memory manager\n");
    
	/*
		The multiboot2 tag is giving and we are storing it in the struct. 
		so we get the memory map easily to know the outline of the memeory
	*/
    vsnx_memory_map_t* mmap = get_vsnx_memory_map();
    if (!mmap || mmap->region_count == 0) {
        printf("PMM: ERROR - No memory map\n");
        return;
    }
    
	/*
		make sure of the memory bounds
	*/
    uint64_t max_addr = 0, min_addr = UINT64_MAX;
    
    for (uint32_t i = 0; i < mmap->region_count; i++) {
        if (mmap->regions[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start = mmap->regions[i].base_addr;
            uint64_t end = start + mmap->regions[i].length;
            if (start < min_addr) min_addr = start;
            if (end > max_addr) max_addr = end;
        }
    }
    
    pmm_info.memory_start = PAGE_ALIGN_DOWN(min_addr);
    pmm_info.memory_end = PAGE_ALIGN(max_addr);
    pmm_info.total_pages = (pmm_info.memory_end - pmm_info.memory_start) / PAGE_SIZE;
    pmm_info.bitmap_size = (pmm_info.total_pages + 7) / 8;
    pmm_info.bitmap = (uint8_t*)0x200000;
    pmm_info.next_alloc_hint = 0;
    
    printf("PMM: Memory 0x%lx-0x%lx (%lu MB)\n",
           pmm_info.memory_start, pmm_info.memory_end,
           (pmm_info.memory_end - pmm_info.memory_start)/(1024*1024));
    
    pmm_info.free_pages = 0;
    pmm_info.used_pages = 0;
    
    printf("PMM: On-demand initialization complete - NO MASSIVE OPERATIONS\n");
}
/*
	Alloc a page simple asf
*/
uint64_t pmm_alloc_page(void) {
    for (uint64_t i = pmm_info.next_alloc_hint; i < pmm_info.total_pages; i++) {
        uint64_t page_addr = pmm_info.memory_start + (i * PAGE_SIZE);
        
        /*
			make sure of the region
		*/
        if (!is_address_available(page_addr)) {
            continue;  // Skip
        }
        
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        
        // Check if page is free in bitmap
        if (!(pmm_info.bitmap[byte_index] & (1 << bit_index))) {
            // Mark as used to avoid reusing
            pmm_info.bitmap[byte_index] |= (1 << bit_index);
            pmm_info.used_pages++;
            pmm_info.next_alloc_hint = i + 1;
            
            return page_addr;
        }
    }
    
    /*
		wrap the search
	*/
    for (uint64_t i = 0; i < pmm_info.next_alloc_hint; i++) {
        uint64_t page_addr = pmm_info.memory_start + (i * PAGE_SIZE);
        
        if (!is_address_available(page_addr)) {
            continue;
        }
        
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        
        if (!(pmm_info.bitmap[byte_index] & (1 << bit_index))) {
            pmm_info.bitmap[byte_index] |= (1 << bit_index);
            pmm_info.used_pages++;
            pmm_info.next_alloc_hint = i + 1;
            
            return page_addr;
        }
    }
    
    return 0;  // Out of memory (immpossible???) vsause moment
}
/*
	Free the page as pleaded inoccent
*/
void pmm_free_page(uint64_t page_addr) {
    if (page_addr < pmm_info.memory_start || page_addr >= pmm_info.memory_end) {
        return;
    }
    
    uint64_t page_index = (page_addr - pmm_info.memory_start) / PAGE_SIZE;
    uint64_t byte_index = page_index / 8;
    uint64_t bit_index = page_index % 8;
    
    if (pmm_info.bitmap[byte_index] & (1 << bit_index)) {
        pmm_info.bitmap[byte_index] &= ~(1 << bit_index);
        pmm_info.used_pages--;
        
        if (page_index < pmm_info.next_alloc_hint) {
            pmm_info.next_alloc_hint = page_index;
        }
    }
}
/*
	markers
*/
void pmm_mark_page_used(uint64_t page_addr) {
    if (page_addr < pmm_info.memory_start || page_addr >= pmm_info.memory_end) {
        return;
    }
    
    uint64_t page_index = (page_addr - pmm_info.memory_start) / PAGE_SIZE;
    uint64_t byte_index = page_index / 8;
    uint64_t bit_index = page_index % 8;
    
    pmm_info.bitmap[byte_index] |= (1 << bit_index);
}
/*
	markers
*/
void pmm_mark_page_free(uint64_t page_addr) {
    if (page_addr < pmm_info.memory_start || page_addr >= pmm_info.memory_end) {
        return;
    }
    
    uint64_t page_index = (page_addr - pmm_info.memory_start) / PAGE_SIZE;
    uint64_t byte_index = page_index / 8;
    uint64_t bit_index = page_index % 8;
    
    pmm_info.bitmap[byte_index] &= ~(1 << bit_index);
}
/*
	get total pages
*/
uint64_t pmm_get_total_pages(void) {
    return pmm_info.total_pages;
}
/*
	Get total free pages
*/
uint64_t pmm_get_free_pages(void) {
    return pmm_info.free_pages;
}
/*
	normal info
*/
void pmm_print_info(void) {
    printf("=== PMM Memory Info ===\n");
    printf("Total pages: %lu (%lu MB)\n", pmm_info.total_pages, 
           (pmm_info.total_pages * PAGE_SIZE) / (1024 * 1024));
    printf("Free pages: %lu (%lu MB)\n", pmm_info.free_pages,
           (pmm_info.free_pages * PAGE_SIZE) / (1024 * 1024));
    printf("Used pages: %lu (%lu MB)\n", pmm_info.used_pages,
           (pmm_info.used_pages * PAGE_SIZE) / (1024 * 1024));
    if (pmm_info.total_pages > 0) {
        printf("Memory utilization: %lu%%\n", 
               (pmm_info.used_pages * 100) / pmm_info.total_pages);
    }
}