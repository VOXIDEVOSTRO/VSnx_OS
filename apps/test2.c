#include "../os/kernel/syscalls/func.h"

/*
    Thread function for testing
	this shi
*/
void thread_worker(void* arg/*googoo gaagaa*/) {
    int tid = (int)(size_t)arg;
    while (1) {
        print("Thread %d running\n", tid);
        //t_sleep(1); // Sleep
    }
}

/*
    Main entry point - Create lots of threads to test scheduler, because i love chaos
*/
void _start(void) {
    print("Creating 100 testing...\n");

    print("All threads executed.");
	t_exit(); // lets exit on our side
}
