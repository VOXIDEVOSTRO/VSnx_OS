/*
    SAFE PAGING SYSTEM
    Minimal on-demand page table management
*/
#include "k_mem.h"
#include "../../../utilities/utility.h"

// Global page manager
vsnx_page_manager_t page_mgr = {0};

/*
    Get page table indices from virtual address
*/
static inline void get_page_indices(uint64_t vaddr, uint64_t* pml4_idx, uint64_t* pdp_idx, uint64_t* pd_idx, uint64_t* pt_idx) {
    *pml4_idx = (vaddr >> 39) & 0x1FF;
    *pdp_idx = (vaddr >> 30) & 0x1FF;
    *pd_idx = (vaddr >> 21) & 0x1FF;
    *pt_idx = (vaddr >> 12) & 0x1FF;
}

/*
    Allocate page table safely
*/
page_table_t* allocate_page_table(void) {
    uint64_t phys_addr = pmm_alloc_page();
    if (!phys_addr) return NULL;
    
    page_table_t* table = (page_table_t*)phys_addr;
    
    /*
		Clear the table
	*/
    for (int i = 0; i < 512; i++) {
        table->entries[i] = 0;
    }
    
    return table;
}

/*
    Initialize paging system
*/
void paging_init(void) {
    printf("PAGING: Initializing safe paging system\n");
    
    /*
		gett he control register 3.
		and using the header.asm page tables
	*/
    uint64_t cr3_value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_value));
    page_mgr.pml4 = (page_table_t*)cr3_value;
    
    printf("PAGING: Using existing PML4 at 0x%lx\n", (uint64_t)page_mgr.pml4);
    
    /*
		And also map the KERNEL SPACE to keep it acsessible thru-out
	*/
    printf("PAGING: Identity mapping kernel space\n");
    for (uint64_t addr = 0; addr < 0x4000000; addr += 0x200000) {
		/*
			Using 2MB granularity for it
			and may only need 2 pages for it because kernel maybe ~1MB and the stack for kernela bout somehere ~16KB
		*/
        if (!paging_map_page_2mb(addr, addr, PAGE_PRESENT | PAGE_WRITABLE)) {
            printf("PAGING: Failed to map kernel page at 0x%lx\n", addr);
            return;
        }
    }
    
    // Flush TLB as normal
    __asm__ volatile("mov %%cr3, %%rax; mov %%rax, %%cr3" ::: "rax", "memory");
    
    printf("PAGING: System ready with kernel mapped\n");
}

// Add 2MB page mapping function
int paging_map_page_2mb(uint64_t vaddr, uint64_t paddr, uint32_t flags) {
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdp_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    
    /*
		get the PDP table
	*/
    if (!(page_mgr.pml4->entries[pml4_idx] & PAGE_PRESENT)) {
        page_table_t* pdp = allocate_page_table();
        if (!pdp) return 0;
        page_mgr.pml4->entries[pml4_idx] = (uint64_t)pdp | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    page_table_t* pdp = (page_table_t*)(page_mgr.pml4->entries[pml4_idx] & ~0xFFF);
    
    /*
		Same for the PD table
	*/
    if (!(pdp->entries[pdp_idx] & PAGE_PRESENT)) {
        page_table_t* pd = allocate_page_table();
        if (!pd) return 0;
        pdp->entries[pdp_idx] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    page_table_t* pd = (page_table_t*)(pdp->entries[pdp_idx] & ~0xFFF);
    
    // Map 2MB page DUH.. 
    pd->entries[pd_idx] = (paddr & ~0x1FFFFF) | flags | PAGE_SIZE_2MB;
    
    return 1;
}

/*
    Map virtual page to physical page
*/
int paging_map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags) {
    uint64_t pml4_idx, pdp_idx, pd_idx, pt_idx;
    get_page_indices(vaddr, &pml4_idx, &pdp_idx, &pd_idx, &pt_idx);
    
    /*
		Get/Create the PDP Table
	*/
    page_table_t* pdp = NULL;
    if (!(page_mgr.pml4->entries[pml4_idx] & PAGE_PRESENT)) {
        pdp = allocate_page_table();
        if (!pdp) return 0;
        page_mgr.pml4->entries[pml4_idx] = (uint64_t)pdp | PAGE_PRESENT | PAGE_WRITABLE | (flags & PAGE_USER);
        page_mgr.total_tables++;
    } else {
        pdp = (page_table_t*)(page_mgr.pml4->entries[pml4_idx] & ~0xFFF);
    }
    
    /*
		same for PD table
	*/
    page_table_t* pd = NULL;
    if (!(pdp->entries[pdp_idx] & PAGE_PRESENT)) {
        pd = allocate_page_table();
        if (!pd) return 0;
        pdp->entries[pdp_idx] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE | (flags & PAGE_USER);
        page_mgr.total_tables++;
    } else {
        pd = (page_table_t*)(pdp->entries[pdp_idx] & ~0xFFF);
    }
    
    /*
		And the PT table
	*/
    page_table_t* pt = NULL;
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
        pt = allocate_page_table();
        if (!pt) return 0;
        pd->entries[pd_idx] = (uint64_t)pt | PAGE_PRESENT | PAGE_WRITABLE | (flags & PAGE_USER);
        page_mgr.total_tables++;
    } else {
        pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);
    }
    
    // Map the page duh...
    pt->entries[pt_idx] = (paddr & ~0xFFF) | flags;
    
    // Flush TLB for this page (TOILET)
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
    
    return 1;
}

/*
    Unmap virtual page
	NOTE: Does not free physical page And common sense
*/
void paging_unmap_page(uint64_t vaddr) {
    uint64_t pml4_idx, pdp_idx, pd_idx, pt_idx;
    get_page_indices(vaddr, &pml4_idx, &pdp_idx, &pd_idx, &pt_idx);
    
    if (!(page_mgr.pml4->entries[pml4_idx] & PAGE_PRESENT)) return;
    page_table_t* pdp = (page_table_t*)(page_mgr.pml4->entries[pml4_idx] & ~0xFFF);
    
    if (!(pdp->entries[pdp_idx] & PAGE_PRESENT)) return;
    page_table_t* pd = (page_table_t*)(pdp->entries[pdp_idx] & ~0xFFF);
    
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return;
    page_table_t* pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);
    
    // Unmap page
    pt->entries[pt_idx] = 0;
    
    // Flush TLB
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

/*
    Get physical address from virtual address
	May be important for the VMM
*/
uint64_t paging_get_physical(uint64_t vaddr) {
    uint64_t pml4_idx, pdp_idx, pd_idx, pt_idx;
    get_page_indices(vaddr, &pml4_idx, &pdp_idx, &pd_idx, &pt_idx);
    
    if (!(page_mgr.pml4->entries[pml4_idx] & PAGE_PRESENT)) return 0;
    page_table_t* pdp = (page_table_t*)(page_mgr.pml4->entries[pml4_idx] & ~0xFFF);
    
    if (!(pdp->entries[pdp_idx] & PAGE_PRESENT)) return 0;
    page_table_t* pd = (page_table_t*)(pdp->entries[pdp_idx] & ~0xFFF);
    
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return 0;
    page_table_t* pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);
    
    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) return 0;
    
    return (pt->entries[pt_idx] & ~0xFFF) | (vaddr & 0xFFF);
}

/*
    Print paging info
	re purposed to test
*/
void paging_print_info(void) {
	printf("VSnx: Testing paging system...\n");	

	uint64_t test_virt = 0x0000700000000000ULL;  // Canonical user space
	uint64_t test_phys = pmm_alloc_page();	

	if (test_phys) {
	    printf("PAGING Test: Mapping 0x%lx -> 0x%lx\n", test_virt, test_phys);
	
	    if (paging_map_page(test_virt, test_phys, PAGE_PRESENT | PAGE_WRITABLE)) {
	        printf("PAGING Test: Mapping successful\n");
		
	        uint64_t resolved = paging_get_physical(test_virt);
	        if (resolved == test_phys) {
	            printf("PAGING Test: Address resolution PASSED (0x%lx)\n", resolved);
			
	            // Test virtual memory access
	            *(uint32_t*)test_virt = 0xCAFEBABE;
	            if (*(uint32_t*)test_virt == 0xCAFEBABE) {
	                printf("PAGING Test: Virtual memory access PASSED\n");
	            } else {
	                printf("PAGING Test: Virtual memory access FAILED\n");
	            }
	        } else {
	            printf("PAGING Test: Address resolution FAILED\n");
	        }
		
	        paging_unmap_page(test_virt);
	    } else {
	        printf("PAGING Test: Mapping FAILED\n");
	    }
	
	    pmm_free_page(test_phys);
	}
}