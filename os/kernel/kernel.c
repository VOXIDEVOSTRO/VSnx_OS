/*
	THE heart of the kernel. The main entry point.
	All the init is handled here. (most of the critical ones)
	All though not much to see here, but still this thing is important.
	cause... its a entry point of the kernel duhh....
*/
/*
	header files
*/
#include "hal/serial_com/serial.h"
#include "hal/vga/vga_text/vga_text.h"
/*
	Some GRUB multiboot2 like for memory mapp
*/
#include "hal/multiboot2/multiboot2.h"
/*
	ISR and IRQ and the entire exception handling
	also the GDT and IDT with ring3 protection and syscalls!
*/
#include "hal/interrupts/interrupts.h"
/*
	Some memory
*/
#include "hal/mem/k_mem/k_mem.h" // PMM
#include "hal/mem/u_mem/u_mem.h" // VMM
/*
	The simple PCI implimentation and enumeration (probably spelled it wrong)
	The AHCI lives here
*/
#include "hal/pci/pci.h"
/*
	Some tasty I/O or disk I/O for filesystem
*/
#include "hal/io/disk/disk.h"
/*
	Some hook stuff
*/
#include "driver/hookcall/hookcall.h"
/*
	Our own printf and its friends like string operations and stuff
*/
#include "utilities/utility.h"
/*
	Some fat32 stuff. like the block interface for Gristle and
	and the gristle headers
*/
#include "fat32/block.h" // Block interface
#include "fat32/gristle.h" // The main defines
/*
	Threading and process management eyecandy.
	Like round robin schedular, and some process isolation 
	in POSIX or linux terms: fork, exec, etc. (No offense to linux users)
*/
#include "threading/thread.h"
#include "elf/process.h" // even i don't have the idea if puting process in elf would've been fine
/*
	Some more POSIX candy. The good ol'
	syscall pal.
*/
#include "syscalls/syscall.h"
/*
	Some GFX driver functions
*/
#include "driver/hookregistry/graphics/gfx.h"
/*
	Keyboard driver functions
*/
#include "driver/hookregistry/hid/keybrd.h"
/*
	Startup configuration parsing
*/
#include "startup/startup.h"
#include "hal/io/ps2/ps2dect.h"
/*
	Clock!
*/
#include "hal/interrupts/clock/pit.h"
/*
	hookorder. Our driver init start!
*/
#include "driver/hookorder/hookorder.h"
/*
	Some uncommentable stuff
*/
//#define TEST		/*UNCOMMENT*/
//#define VGA		/*UNCOMMENT*/
/*

	Simple test

*/
#ifdef TEST
/*
	Small crude delay for test
*/
static void delay(volatile int count) {
    for (volatile int i=0; i<count*100000; i++) { __asm__ __volatile__("nop");/*crude*/}
}

void dvd_screensaver(void) {
    int x = 100, y = 100;
    int dx = 3, dy = 2;
    int w = 1024, h = 768; /*screen*/
    int logo_w = 100, logo_h = 100; /*Logo bound box*/

    uint32_t colors[] = {4};
    int color_index = 0;
	/*
		Simple animation...
	*/
    for (;;) {
        gfx_clear(0x000000);
		/*
			make the ball
		*/
        gfx_fill_rect(x, y, logo_w, logo_h, colors);
		/*
			Update
		*/
        x += dx; y += dy;
		/*
			Bounce
		*/
        if (x <= 0 || x + logo_w >= w) { dx = -dx; color_index = (color_index); }
        if (y <= 0 || y + logo_h >= h) { dy = -dy; color_index = (color_index); }

        delay(100);
    }
}

void worker_thread(void* arg) {
    while (1) {
		/*
			Some stuff
		*/
        uint32_t has_key = 0;
        uint32_t keycode = 0;
        uint32_t unicode = 0;
        uint32_t mods = 0;
		/*
			Simple getchar loop
		*/
        kbd_has_key(&has_key);
        if (has_key) {
			/*
				handle the key
			*/
            kbd_get_key(&keycode);
            kbd_translate(keycode, &unicode);
            kbd_get_modifiers(&mods);
			/*
				Printup if a valid ASCII
			*/
            if (unicode >= 32 && unicode <= 126) {
                printf("%c", (char)unicode);
            } else if (unicode == 13) {
                printf("\n");
            }
        }
    }
}
#endif
/*

	Post init for other stuff
	because we would have the hookorder here and when
	even one thread is active, the kernel will lock itself
	to the interrupt handler meaning nothing after the first thread.
	so making kernel worker thread

*/
void post_init_thread(void* arg) {
    ps2_devices_init();
	#ifdef TEST
	thread_t* worker = thread_create(worker_thread/*me stupid*/, NULL, THREAD_RING0, THREAD_PRIORITY_IMMEDIATES/*no need*/, 0);
	if (worker) {
		if (thread_execute(worker) == 0) {
			#ifdef DEBUG
			printf("VSnx: test thread started\n");
			#endif
		} else {
			#ifdef DEBUG
			printf("VSnx: failed\n");
			#endif
		}
	} else {
		#ifdef DEBUG
		printf("VSnx: Failed to create the work thread\n");
		#endif
	}
	#endif
	/*
		START THE DRIVERS !
		Main init for the drivers
		via the hook order
	*/
	start_hookorder();

    // Call startup config parser to spawn processes
    #ifdef DEBUG
    printf("KERNEL: Parsing startup process list...\n");
    #endif
	/*
		LOAD the start up apps
		simple config parsing
	*/
    parse_startup_list();
	/*
		VBOX svga test
	*/
	#ifdef TEST
	dvd_screensaver(/*satisfying*/);
	#endif
	/*
		Done with init	
	*/
	thread_exit();
}
/*
	MAIN KERNEL ENTRY POINT
	The starting point of the kernel
*/
void kernel_main(uint64_t multiboot_info_addr/*Passing the mb2 Addr. because we need multiboot2 tag for memory map*/) {
	/*
		The first and foremost
		the serial output is initialized 
		here SO we have an idea what code
		destroyed itself or kernel
	*/
	#ifdef DEBUG // just to avoid clutter. to change this: go to the utilities header file
    serial_init(COM1_PORT);
    serial_print(COM1_PORT, "VSnx Kernel Starting...\r\n"); // Simplest possible
	#endif
	/*
		Now the VGA! because why not (text)
		also we detect if VGA text there.
	*/
    if(vga_text_init() == 0) {
		#ifdef DEBUG
		printf("VSnx Kernel - VGA Active!\n");
		#endif
		/*
			The reason for dual output. because i am lazy to check the Serial everytime
			or just i thought of it as a great idea to not keep the screen empty LOL
		*/
		#ifdef DEBUG
		printf("VSnx: Dual output (Serial + VGA) ready\n");
		#endif
	} else {
		/*
			Incase No VGA text here
		*/
		#ifdef DEBUG
		printf("VSnx: Serial-only mode\n");
		#endif
	}
	printf("VSnx booted...Starting init\n");
	/*
		making sure we have PIT at 1000hz meaning 1ms
		because of clock
	*/
	set_freq(1000);
	/*
		time for the memory map. and important for AMD64 system or x86_64.
		As we need to use as much memory we can. And to get the picture.
		We just pass the mb2 addr as it is and map and parse to get the tag
		so we can put it in our vsnx memory map structure. so that the PMM can
		track on demand memory allocation and deallocation. This is why kernel has
		the argumnet so we can easily pass on here.
	*/
	if(multiboot2_parse_memory_map(multiboot_info_addr) == 0) {
		#ifdef DEBUG
		printf("VSnx: Memory map parsed successfully\n");
		#endif
		/*
			Just for debug. what more can i say
		*/
		print_memory_map();
	} else {
		#ifdef DEBUG
		printf("VSnx: Memory map parsing failed\n");
		#endif
		return; // YOU FAILURE!!!!!!!
	}
	/*
		Now comes the very critical. the GDT and IDT. the main handlers
		of the interrupts. and quite simple yet effectively. we have ring3
		setup. and the most important of all the syscalls and the exception
		handling lie here (ISR). and main handling (IRQ) for like hardware
		interrupts like keyboard, PS2 mouse etc... but software ones too like
		the syscalls.
	*/
	#ifdef DEBUG
	printf("VSnx: Initializing interrupt system...\n");
	#endif
	/*
		Init the GDT
	*/
	gdt_init();
	/*
		init the IDT
	*/
	idt_init();
	#ifdef DEBUG
	printf("VSnx: Initializing Physical Memory Manager...\n");
	#endif
	/*
		Now the star of the show. the PMM (Physical memory manager)
		And what i say is kernel memory. becuase kernel has the most
		acsess to it. its a fairly simple and also VMM is based on top
		of the PMM and dependent.
	*/
	pmm_init();
	/*
		print the random statistics. (probably spelled it wrong)
		just for debug
	*/
	pmm_print_info();
	/*
		Here is the paging. and more important if viwed from the VMM's
		perspective (Virtual memory manager). We make virtual memory like size of 256TB
		and which is pretty massive but ALL of this FOLLOW one thing common. On demand mapping
		we dont map uneccesary memory in any of these steps.
	*/
	#ifdef DEBUG
	printf("VSnx: Initializing paging system...\n");
	#endif
	paging_init();
	paging_print_info();
	#ifdef DEBUG
	printf("VSnx: Initializing VMM...\n");
	#endif
	vmm_init();
	vmm_print_info();
	/*
		and PCI. another important thing. (for modern systems)
		and this holds like USB, network, audio, GPU, AHCI, and other things.
		For now we only have AHCI or SATA disk control tied to it.
	*/
	#ifdef DEBUG
	printf("VSnx: Initializing PCI...\n");
	#endif
	pci_driver_init();
	/*
		Some I/O candy.
		detect the disk. becuase we need to know it so we don't write to a phantom
		or some broken stuff.
	*/
	#ifdef DEBUG
	printf("VSnx: Detecting the disk...\n");
	#endif
	disk_detection_init();
	/*
		Also init it
	*/
	#ifdef DEBUG
	printf("VSnx: Initializing disk...\n");
	#endif
	disk_init();
	/*
		And storage controllers. Our OS currently on has
		AHCI/SATA and IDE/ATA. but we can add more in future
	*/
	disk_detection_init();

	/*
		So now detect the disk on the controller. (i dont think we should do this twice)
	*/
	int total_disks = disk_detect_all();

	if (total_disks > 0) {
		/*
			pRINTOUT THE INFORMATION 
		*/
	    disk_print_info();
		/*
			Also do a test to make sure if the disk is acsessible and working
		*/
	    #ifdef DEBUG
	    printf("VSnx: Testing disk I/O...\n");
	    #endif
	    int test_result = disk_test_io(0);
	    if (test_result == 0) {
	        #ifdef DEBUG
	        printf("VSnx: Disk I/O test PASSED\n");
	        #endif
	    } else {
			#ifdef DEBUG
	        printf("VSnx: Disk I/O test FAILED (error %d)\n", test_result); // erro
			#endif
		}
	} else {
	    #ifdef DEBUG
	    printf("VSnx: No disks detected\n");
	    #endif
	}

	/*
		Now here comes the FILESYSTEM
		The FAT32. the most compatibale Filesystem. and thanks from help of gristle
		we init its block device. and mount it. duh....
	*/
	if (total_disks > 0) {
	    #ifdef DEBUG
	    printf("VSnx: Initializing block device...\n");
	    #endif
		/*
			init the block interface. 
		*/
	    if (block_init() == 0) {
	        #ifdef DEBUG
	        printf("VSnx: Block device ready\n");
	        #endif
			/*
				Mount the filesystem
			*/
	        #ifdef DEBUG
	        printf("VSnx: Mounting FAT filesystem...\n");
	        #endif
	        blockno_t volume_size = block_get_volume_size();
	        if (fat_mount(0, volume_size, 0) == 0) {
	            #ifdef DEBUG
	            printf("VSnx: FAT filesystem mounted successfully!\n");
	            #endif
			
	        } else {
	            #ifdef DEBUG
	            printf("VSnx: Failed to mount FAT filesystem\n");
	            #endif
	        }
	    } else {
	    }
	} else {
		/*
			Incase something wrong
		*/
		#ifdef DEBUG
		printf("VSnx: No disks detected\n");
		#endif
	}
	/*
		So the most important. The threading/multithreading. and process management or tracking.
		Absoulte beast of thing here. thread make the fabric of this OS or any process... press F to pay
		respect
	*/
	#ifdef DEBUG
	printf("VSnx: Initializing threading system...\n");
	#endif
	threading_init();
	/*
		user space syscalls
	*/
	init_syscalls();
	/*
		And our driver interface
	*/
	init_hookcalls();
	/*
		Start of the post init, and handled by a kernel worker thread.
	*/
	#ifdef DEBUG
	printf("VSnx: Creating post init worker thread...\n");
	#endif
	/*
		The reason to use thread here is because if even one thread becomes active the kernel hits the dead end,
		because the schedular isn't gonna round robin the kernel it self, as its not a valid PID and TID,
		hence, we will make kernel worker thread which will sceduled and properly handle the drivers,
		load them and execute, them all of them. I am just stupid to troubleshoot a problem which would 
		have been solved by existing systems... wasted like 4 hours...
	*/
	thread_t* driver_worker = thread_create(post_init_thread, NULL, THREAD_RING0, THREAD_PRIORITY_IMMEDIATES/*finish as soon as possible*/, 0);
	if (driver_worker) {
		/*
			EXECUTE the thread
		*/
		if (thread_execute(driver_worker) == 0) {
			#ifdef DEBUG
			printf("VSnx: Post init started\n");
			#endif
		} else {
			#ifdef DEBUG
			printf("VSnx: Post init worker failed\n");
			#endif
		}
	} else {
		#ifdef DEBUG
		printf("VSnx: Failed to create the post thread\n");
		#endif
	}
	#ifdef DEBUG
	/*
		Prolly won't see this message
	*/
	printf("VSnx: Done with init\n");
	#endif
    while (1) { /*main idle*/
		/*
			Void of a loop
		*/
        __asm__ volatile("hlt"/*wait up for int*/);
    }
}