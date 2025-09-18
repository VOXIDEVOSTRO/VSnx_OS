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
	/*
		The chaos starts here
	*/
	t_sleep(10);
    int num_threads = 100;
    int tids[num_threads];
    int created = 0;
    for (int i = 0; i < num_threads; i++) {
        int priority;
        if (i < 10) {
            priority = THREAD_PRIORITY_HIGH; 
        } else if (i < 50) {
            priority = THREAD_PRIORITY_NORMAL;
        } else {
            priority = THREAD_PRIORITY_LOW;
        }
		/*
			Create those threads
		*/
        int tid = t_create((void*)thread_worker, (void*)(size_t)i, 3, priority, getpid());
        if (tid > 0) {
            tids[created++] = tid;
            print("Created thread %d with priority %d\n", tid, priority);
        } else {
            print("Failed to create thread %d\n", i);
        }
    }
	/*
		Notify
	*/
    print("Created %d threads total\n", created);
    for (int i = 0; i < created; i++) {
        if (t_execute(tids[i]) != 0) {
            print("Failed to execute thread %d\n", tids[i]);
        }
    }

    print("All threads executed.");
	t_exit(); // lets exit on our side
	while(1) {/*Loop of idleness*/}
}
