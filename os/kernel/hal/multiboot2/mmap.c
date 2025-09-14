/*
	Header files
*/
#include "multiboot2.h"
/*
	Declaring mmap globals
*/
vsnx_memory_map_t vsnx_mmap = {0};
/*
	PARSE m2 mmap
*/
int multiboot2_parse_memory_map(uint64_t multiboot2_addr) {
	/*
		get the tag and m2 addr
	*/
    multiboot2_info_t* mbi = (multiboot2_info_t*)multiboot2_addr;
    multiboot2_tag_t* tag;
    
    #ifdef DEBUG
    printf("MULTIBOOT2: Parsing memory map...\n");
    #endif
    
    /*
		Start with the fix part
	*/
    tag = (multiboot2_tag_t*)(multiboot2_addr + 8);
    
	/*
		Parse tag
	*/
    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            multiboot2_tag_mmap_t* mmap_tag = (multiboot2_tag_mmap_t*)tag;
            uint32_t entry_count = (mmap_tag->size - sizeof(multiboot2_tag_mmap_t)) / mmap_tag->entry_size;
            
            #ifdef DEBUG
            printf("MULTIBOOT2: Found memory map with %u entries\n", entry_count);
            #endif
            
			/*
				Initilize
			*/
            vsnx_mmap.region_count = 0;
            vsnx_mmap.total_memory = 0;
            vsnx_mmap.usable_memory = 0;
            
			/*
				Get the memory map entries
			*/
            for (uint32_t i = 0; i < entry_count && i < 256; i++) {
                multiboot2_mmap_entry_t* entry = &mmap_tag->entries[i];
                
                vsnx_mmap.regions[i].base_addr = entry->addr;
                vsnx_mmap.regions[i].length = entry->len;
                vsnx_mmap.regions[i].type = entry->type;
                vsnx_mmap.regions[i].reserved = 0;
                
                vsnx_mmap.total_memory += entry->len;
                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    vsnx_mmap.usable_memory += entry->len;
                }
                
                vsnx_mmap.region_count++;
            }
            
            #ifdef DEBUG
            printf("MULTIBOOT2: Memory map parsed successfully\n");
            #endif
            #ifdef DEBUG
            printf("MULTIBOOT2: Total memory: %lx bytes (%lu MB)\n", vsnx_mmap.total_memory, vsnx_mmap.total_memory / (1024*1024));
            #endif
            #ifdef DEBUG
            printf("MULTIBOOT2: Usable memory: %lx bytes (%lu MB)\n", vsnx_mmap.usable_memory, vsnx_mmap.usable_memory / (1024*1024));
            #endif
            return 0;
        }
        
        // Move to next tag (align to 8 bytes) alignment is critical here
        tag = (multiboot2_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    #ifdef DEBUG
    printf("MULTIBOOT2: No memory map found\n");
    #endif
    return -1; // error
}
/*
	Print detailed memory map
*/
void print_memory_map(void) {
    #ifdef DEBUG
    printf("=== VSnx Memory Map ===\n");
    #endif
    #ifdef DEBUG
    printf("Regions: %u\n", vsnx_mmap.region_count);
    #endif
    
    for (uint32_t i = 0; i < vsnx_mmap.region_count; i++) {
        const char* type_str;
        switch (vsnx_mmap.regions[i].type) {
            case MULTIBOOT_MEMORY_AVAILABLE: type_str = "Available"; break;
            case MULTIBOOT_MEMORY_RESERVED: type_str = "Reserved"; break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE: type_str = "ACPI Reclaim"; break;
            case MULTIBOOT_MEMORY_NVS: type_str = "ACPI NVS"; break;
            case MULTIBOOT_MEMORY_BADRAM: type_str = "Bad RAM"; break;
            default: type_str = "Unknown"; break;
        }
        
        #ifdef DEBUG
        printf("Region %u: 0x%lx - 0x%lx (%s)\n", i, vsnx_mmap.regions[i].base_addr, vsnx_mmap.regions[i].base_addr + vsnx_mmap.regions[i].length - 1, type_str);
        #endif
    }
}
/*
	API for getting the mmap
*/
vsnx_memory_map_t* get_vsnx_memory_map(void) {
    return &vsnx_mmap;
}

uint64_t get_total_memory(void) {
    return vsnx_mmap.total_memory;
}

uint64_t get_usable_memory(void) {
    return vsnx_mmap.usable_memory;
}
