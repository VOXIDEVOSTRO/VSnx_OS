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
	And the Keyboard (ps2)
*/
#include "hal/io/ps2/ps2.h"
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
	Some drivers
*/
#include "hal/vga/vga_gfx/vga_gfx.h"
/*
	Some GFX driver functions
*/
#include "driver/hookregistry/graphics/gfx.h"
/*
	Startup configuration parsing
*/
#include "startup/startup.h"
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
	/*
		NOW init the keyboard or PS2
	*/
	/*
		PS/2
	*/
	#ifdef DEBUG
	printf("VSnx: Detecting PS/2 devices...\n");
	#endif
	/*
		INIT some vars
	*/
	ps2_controller.dual_channel = false;
	ps2_controller.port1_exists = false;
	ps2_controller.port2_exists = false;
	ps2_controller.port1_device = PS2_DEVICE_NONE;
	ps2_controller.port2_device = PS2_DEVICE_NONE;
	ps2_controller.initialized = false;
	/*
		FIRST disable
	*/
	ps2_write_command(PS2_CMD_DISABLE_PORT1);
	ps2_write_command(PS2_CMD_DISABLE_PORT2);

	/*
		FLUSH (not the toilet)
	*/
	while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
	    inb(PS2_DATA_PORT);
	}

	/*
		Set the configuration
	*/
	ps2_write_command(PS2_CMD_READ_CONFIG);
	uint8_t config = ps2_read_data();
	ps2_controller.config_byte = config;
	#ifdef DEBUG
	printf("VSnx: PS/2 original config: 0x%02X\n", config);
	#endif
	/*
		Check if dual
	*/
	ps2_controller.dual_channel = (config & PS2_CONFIG_PORT2_CLOCK) != 0;

	config &= ~(PS2_CONFIG_PORT1_INT | PS2_CONFIG_PORT2_INT | PS2_CONFIG_PORT1_TRANS);
	ps2_write_command(PS2_CMD_WRITE_CONFIG);
	ps2_write_data(config);
	/*
		do a self test
	*/
	ps2_write_command(PS2_CMD_TEST_CONTROLLER);
	uint8_t test_result = ps2_read_data();

	if (test_result != 0x55) {
		#ifdef DEBUG
	    printf("VSnx: PS/2 controller self-test failed: 0x%02X\n", test_result);
		#endif
	} else {
		#ifdef DEBUG
	    printf("VSnx: PS/2 controller self-test passed\n");
		#endif
		/*
			test ports
		*/
	    ps2_write_command(PS2_CMD_TEST_PORT1);
	    test_result = ps2_read_data();
	    ps2_controller.port1_exists = (test_result == 0x00);
		#ifdef DEBUG
	    printf("VSnx: PS/2 Port 1 test: 0x%02X (%s)\n", test_result, 
	           ps2_controller.port1_exists ? "OK" : "FAIL");
		#endif
		/*
			IF dual channel. Enable
		*/
	    if (ps2_controller.dual_channel) {
	        ps2_write_command(PS2_CMD_TEST_PORT2);
	        test_result = ps2_read_data();
	        ps2_controller.port2_exists = (test_result == 0x00);
			#ifdef DEBUG
	        printf("VSnx: PS/2 Port 2 test: 0x%02X (%s)\n", test_result,
	               ps2_controller.port2_exists ? "OK" : "FAIL");
			#endif
		}
		/*
			IDENTIFY
		*/
	    if (ps2_controller.port1_exists) {
	        ps2_write_command(PS2_CMD_ENABLE_PORT1);
			/*
				Check the device on 1
			*/
	        ps2_controller.port1_device = ps2_identify_device(1);
		
	        if (ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_OLD ||
	            ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_MF2) {
				#ifdef DEBUG
	            printf("VSnx: Keyboard detected on port 1, initializing...\n");
				#endif
	            /*
					FINALLY init the keyboard
				*/
	            ps2_keyboard_init(1);
				#ifdef DEBUG
	            printf("VSnx: Keyboard ready for input\n");
				#endif
	        } else {
				#ifdef DEBUG
	            printf("VSnx: No keyboard on port 1 (device type: %d)\n", ps2_controller.port1_device);
				#endif
	        }
	    }
		/*
			ALSO check the 2
		*/
	    if (ps2_controller.port2_exists) {
	        ps2_write_command(PS2_CMD_ENABLE_PORT2);
			/*
				IDENTITFY
			*/
	        ps2_controller.port2_device = ps2_identify_device(2);
			/*
				To be FIXED or added:
				Add mouse init here
			*/
	        if (ps2_controller.port2_device >= PS2_DEVICE_MOUSE_STANDARD) {
				#ifdef DEBUG
	            printf("VSnx: Mouse detected on port 2\n");
				#endif
	        } else {
				#ifdef DEBUG
	            printf("VSnx: No mouse on port 2 (device type: %d)\n", ps2_controller.port2_device);
				#endif
	        }
	    }
		/*
			Enable hardware interrupts for them
		*/
	    ps2_write_command(PS2_CMD_READ_CONFIG);
	    config = ps2_read_data();
	
	    if (ps2_controller.port1_exists && 
	        (ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_OLD ||
	         ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_MF2)) {
	        config |= PS2_CONFIG_PORT1_INT;
			#ifdef DEBUG
	        printf("VSnx: Enabled keyboard interrupts (IRQ1)\n");
			#endif
	    }
	
	    if (ps2_controller.port2_exists && 
	        ps2_controller.port2_device >= PS2_DEVICE_MOUSE_STANDARD) {
	        config |= PS2_CONFIG_PORT2_INT;
			#ifdef DEBUG
	        printf("VSnx: Enabled mouse interrupts (IRQ12)\n");
			#endif
	    }
		/*
			we are done
		*/
	    ps2_write_command(PS2_CMD_WRITE_CONFIG);
	    ps2_write_data(config);
	
	    ps2_controller.initialized = true;
		#ifdef DEBUG
	    printf("VSnx: PS/2 initialization complete\n");
	    printf("VSnx: Port 1 device: %d, Port 2 device: %d\n", 
	           ps2_controller.port1_device, ps2_controller.port2_device);
		#endif
	}
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
	pci_init();
	pci_print_devices();	
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
		just after it... THE SYSCALLS. standard of communication between kernel and thread/process for some
		previlaged acsess (probably spelled it wrong)
	*/
	init_syscalls();
	init_hookcalls();
	/*
		THEN first try to check for a VGA graphics
		Both modes are supported: 13h and 12h
	*/



	/*
	
		UN COMMENT if we want to test VGA.
		For vga driver
	
	*/

	//#define VGA /*UNCOMMENT IF NEED VGA!*/

	#ifdef VGA
	vga_load_graphics_driver(/*Load some drivers, In this case its VGA*/);
	#endif
	#ifdef DEBUG
	printf("VSnx: Done\n");
	#endif
    // Call startup config parser to spawn processes
    #ifdef DEBUG
    printf("KERNEL: Parsing startup process list...\n");
    #endif
	/*
		LOAD the start up apps
		simple config parsing
	*/
    parse_startup_list();
    while (1) { /*main idle*/
		/*
			Void of a loop
		*/
		/*
			ALSo here is demostration of the drivers and thier interfaces
		*/
		#ifdef VGA
		gfx_fill_rect(20,  20,  80, 60, 4);   
        gfx_fill_rect(120, 20,  80, 60, 2);
        gfx_fill_rect(220, 20,  80, 60, 1); 
        gfx_fill_rect(100, 100, 120, 80, 15);
		#endif
        __asm__ volatile("hlt"); // wait for interrupt
    }
}