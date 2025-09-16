#include "../os/kernel/syscalls/func.h"

/*
    Main entry point
*/
void _start(void) {
    print("hello BS stupid!\n");
    
	while (1) {
        __asm__ volatile("nop");
		//print("hello BS stupid! SRYY from test2\n");
    }
}