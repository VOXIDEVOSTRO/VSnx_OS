#include "../os/kernel/syscalls/func.h"

/*
    Main entry point
*/
void _start(void) {
    print("hello BS from test1\n");
    
	//t_exit();
	//print("FAILURE YOU FAILURE CRITICAL"); // prbably shouldnt see this
	while (1) {
        __asm__ volatile("nop");
		print("hello stupid! also sry from test1\n");
    }
}