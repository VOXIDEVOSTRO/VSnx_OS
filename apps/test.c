#include "../os/kernel/syscalls/func.h"

/*
    Main entry point
*/
void _start(void) {
    print("hello BS but testing\n");
    
	while (1) {
        __asm__ volatile("nop");
		print("hello stupid! BUT FRROM first test\n");
    }
}