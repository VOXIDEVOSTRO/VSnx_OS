/*
	Header files
*/
#include "../os/kernel/syscalls/func.h"
/*
	This is the PIPE
*/
void pipetest(void* data) {
	/*
		Unpack
	*/
    struct pipe_data* pdata = (struct pipe_data*)data;
    const char* received_msg = (const char*)pdata->args[0];
    print("IPC Handler received message: ");
    print(received_msg);
}

/*
    Main entry point
	Some IPC pipe demo\

	ALSO IGNORE THESE. because just test artifacts of the IPC

*/
void _start(void) {
	/*
		Just create the pipe
	*/
    int res = createpipe("testpipe", pipetest);
    if (res != 0) {
        print("Failed to pipe up");
    } else {
        print("Pipe created");
    }
	/*
		Loop of IDLENESS
	*/
    while (1) {
		print("RUNNING T");
        __asm__ volatile("nop");
    }
}
