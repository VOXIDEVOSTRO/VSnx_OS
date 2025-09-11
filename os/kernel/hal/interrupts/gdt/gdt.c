/*
	HEADER FIELS
*/
#include "gdt.h"
/*
	Declaring globals
*/
gdt_entry_t gdt_entries[7];
tss_entry_t tss_entry;
gdt_ptr_t gdt_ptr;
tss_t tss;
/*
    GDT Flush using inline assembly
	simple you know
*/
void gdt_flush_inline(void) {
    __asm__ volatile(
        "lgdt %0\n\t"           // Load GDT
        "mov $0x10, %%ax\n\t"   // Kernel data selector
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        "pushq $0x08\n\t"       // Kernel code selector
        "pushq $1f\n\t"         // Push return address
        "lretq\n\t"             // Far return to reload CS
        "1:\n\t"
        :
        : "m"(gdt_ptr)
        : "rax", "memory"
    );
}
/*
    GDT set any entry
*/
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[index].base_low    = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high   = (base >> 24) & 0xFF;
    gdt_entries[index].limit_low   = (limit & 0xFFFF);
    gdt_entries[index].granularity = (limit >> 16) & 0x0F;
    gdt_entries[index].granularity |= gran & 0xF0;
    gdt_entries[index].access = access;
}

/*
    GDT set TSS entry (64-bit)
*/
void gdt_set_tss_entry(int index, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    /*
		TSS takes 2 GDT entries in 64-bit mode
	*/
    gdt_entries[index].limit_low   = limit & 0xFFFF;
    gdt_entries[index].base_low    = base & 0xFFFF;
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].access      = access;
    gdt_entries[index].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[index].base_high   = (base >> 24) & 0xFF;
    /*
		The second entry
	*/
    gdt_entries[index + 1].limit_low   = (base >> 32) & 0xFFFF;
    gdt_entries[index + 1].base_low    = (base >> 48) & 0xFFFF;
    gdt_entries[index + 1].base_middle = 0;
    gdt_entries[index + 1].access      = 0;
    gdt_entries[index + 1].granularity = 0;
    gdt_entries[index + 1].base_high   = 0;
}
/*
    Initialize TSS
*/
void tss_init(void) {
    // Clear TSS
    for (int i = 0; i < sizeof(tss_t); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }
    
    tss.rsp0 = 0x200000;  // 2MB kernel stack (i dont remember the stack size)
    tss.iomap_base = sizeof(tss_t);
    
    printf("TSS: Task State Segment initialized at 0x%lx\n", (uint64_t)&tss);
}

/*
    SETUP ALL GDT ENTRIES
*/
void gdt_init(void) {
    printf("GDT: Initializing Global Descriptor Table...\n");
    
    // TSS needs 2 entries, so we need 7 total entries
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 7) - 1;
    gdt_ptr.base = (uint64_t)&gdt_entries;
    
    // Set up GDT entries becuase yes
    gdt_set_entry(0, 0, 0, 0, 0);  // NULL (NULL descriptor)
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0);  // Kernel code
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0x80);  // Kernel data
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xA0);  // User code (Ring 3)
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0x80);  // User data (Ring 3)
    
	/*
		Some TSS setup here
	*/
    tss_init();
    gdt_set_tss_entry(5, (uint64_t)&tss, sizeof(tss_t) - 1, 0x89, 0x00);
    
    printf("GDT: Loading GDT and TSS...\n");
    
    /*
		load the GDT (to CPU?)
	*/
    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
    
    /*
		Load the TSS
	*/
    __asm__ volatile("ltr %0" : : "r"((uint16_t)TSS_SELECTOR));
    
    printf("GDT: TSS loaded successfully\n");
}
