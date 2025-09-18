/*
    Header files
*/
#include <stdint.h>
#include "../os/kernel/syscalls/func.h"
/*
	test junk
*/
#define BUFFER_SIZE 4096
#define TEST_FILE "test.txt"
#define TEST_DIR "testdir"
#define ITERATIONS 1000

/*
    Main entry point
*/
void _start(void) {

	print("HELLO starting busy waits");

	print("Starting first busy wait...");
	for (volatile long long i = 0; i < 1000000000LL; i++) {
	}
	print("First busy wait completed");

	// Second massive busy wait
	print("Starting second busy wait...");
	for (volatile long long i = 0; i < 1000000000LL; i++) {
	}
	print("Second busy wait completed");
	t_sleep(15);
    print("Busy waits completed\n");
	/*
		Idle up
	*/
    while (1) {
		print("RUNNING TTTTTTT");
        __asm__ volatile("nop");
    }
}
