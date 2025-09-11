/*
	header files
*/
#include "hal/serial_com/serial.h"
#include "hal/vga/vga_text/vga_text.h"
#include "hal/multiboot2/multiboot2.h"
#include "hal/interrupts/interrupts.h"
#include "hal/mem/k_mem/k_mem.h"
#include "hal/mem/u_mem/u_mem.h"
#include "hal/pci/pci.h"
#include "hal/io/disk/disk.h"
#include "utilities/utility.h"
#include "fat32/block.h"
#include "fat32/gristle.h"
#include "threading/thread.h"
/*
	SMALL test
*/
void user_thread1(void* arg) {
    static int count = 0;
    while (1) {
        count++;
        if (count % 1000000 == 0) {
            printf("USER THREAD 1: Running! Count=%d\n", count);
        }
        // Small delay to prevent spam
        for (int i = 0; i < 100000; i++) __asm__ volatile("nop");
    }
}

void user_thread2(void* arg) {
    static int count = 0;
    while (1) {
        count++;
        if (count % 1000000 == 0) {
            printf("USER THREAD 2: Running! Count=%d\n", count);
        }
        // Small delay to prevent spam
        for (int i = 0; i < 100000; i++) __asm__ volatile("nop");
    }
}
/*
	MAIN KERNEL ENTRY POINT
*/
void kernel_main(uint64_t multiboot_info_addr) {

    serial_init(COM1_PORT);
    serial_print(COM1_PORT, "VSnx Kernel Starting...\r\n");

    if(vga_text_init() == 0) {
		vga_println("VSnx Kernel - VGA Active!");
		printf("VSnx: Dual output (Serial + VGA) ready\n");
	} else {
		printf("VSnx: Serial-only mode\n");
	}
	
	if(multiboot2_parse_memory_map(multiboot_info_addr) == 0) {
		printf("VSnx: Memory map parsed successfully\n");
		print_memory_map(); // Show detailed memory info
	} else {
		printf("VSnx: Memory map parsing failed\n");
		return;
	}
	
	printf("VSnx: Initializing interrupt system...\n");
	
	gdt_init();
	
	idt_init();
	
	printf("VSnx: Initializing Physical Memory Manager...\n");

	// Initialize PMM
	pmm_init();
	
	// Show final memory stats
	pmm_print_info();

	printf("VSnx: Initializing paging system...\n");
	paging_init();
	paging_print_info();
	
	printf("VSnx: Initializing VMM...\n");
	vmm_init();
	vmm_print_info();

	printf("VSnx: Initializing PCI subsystem...\n");
	pci_init();
	pci_print_devices();	

	printf("VSnx: Detecting the disk...\n");
	disk_detection_init();

	printf("VSnx: Initializing disk subsystem...\n");

	// Initialize disk management
	disk_init();

	// Detect storage controllers
	disk_detection_init();

	// Detect and register all disks
	int total_disks = disk_detect_all();

	if (total_disks > 0) {
	    // Print disk information
	    disk_print_info();
	
	    // Test first disk
	    printf("VSnx: Testing disk I/O...\n");
	    int test_result = disk_test_io(0);
	    if (test_result == 0) {
	        printf("VSnx: Disk I/O test PASSED\n");
	    } else {
	        printf("VSnx: Disk I/O test FAILED (error %d)\n", test_result);
	    }
	} else {
	    printf("VSnx: No disks detected\n");
	}

	
	if (total_disks > 0) {
	    // Initialize block device layer
	    printf("VSnx: Initializing block device...\n");
	    if (block_init() == 0) {
	        printf("VSnx: Block device ready\n");
		
	        // Initialize FAT filesystem
	        printf("VSnx: Mounting FAT filesystem...\n");
		
	        // Mount filesystem: start at sector 0, use full disk size, auto-detect type
	        blockno_t volume_size = block_get_volume_size();
	        if (fat_mount(0, volume_size, 0) == 0) {
	            printf("VSnx: FAT filesystem mounted successfully!\n");
			
	        } else {
	            printf("VSnx: Failed to mount FAT filesystem\n");
	        }
	    } else {
	    }
	} else {
		printf("VSnx: No disks detected\n");
	}

	printf("VSnx: Initializing threading system...\n");
	threading_init();

	printf("VSnx: Testing threading system...\n");
		
	thread_t* thread1 = thread_create(user_thread1, NULL, THREAD_RING0, THREAD_PRIORITY_NORMAL);
	thread_t* thread2 = thread_create(user_thread2, NULL, THREAD_RING0, THREAD_PRIORITY_NORMAL);
	
	if (thread1 && thread2) {
	    printf("THREADING: Created threads TID=%d, TID=%d\n", thread1->tid, thread2->tid);
	
	    // Execute threads
	    thread_execute(thread1);
	    thread_execute(thread2);
	
	    printf("THREADING: Threads started with VMM-allocated memory\n");
	
	}

	printf("VSnx: Kernel initialization complete!\n");
	
    // Kernel idling
    while (1) {
        __asm__ volatile("hlt"); // wait for interrupt
    }
}