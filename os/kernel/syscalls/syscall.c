/*

	Header files

*/
#include "syscall.h"
#include "handles.h"
/*

	MAIN HANDLER

*/
void syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
	if (syscall_num >= MAX_SYSCALLS || syscall_table[syscall_num].handler == NULL) {
 		__asm__ volatile("movq $-1, %%rax" : : : "rax");
 		return; // dah
 	}
 	int64_t result = syscall_table[syscall_num].handler(arg1, arg2, arg3, arg4, arg5, arg6);
 	__asm__ volatile("movq %0, %%rax" : : "r"(result) : "rax"); // returns as usual
}
/*

	ASSEMBLY BLOCK For handling the syscall interrupt

*/
__asm__(
	".global syscall_entry\n"
	"syscall_entry:\n"
	" # Save all registers\n"
	/*		
		Save the registers
	*/
	" pushq %rbx\n"
	" pushq %rcx\n"
	" pushq %rdx\n"
	" pushq %rsi\n"
	" pushq %rdi\n"
	" pushq %rbp\n"
	" pushq %r8\n"
	" pushq %r9\n"
	" pushq %r10\n"
	" pushq %r11\n"
	" pushq %r12\n"
	" pushq %r13\n"
	" pushq %r14\n"
	" pushq %r15\n"
	" \n"
	" # Arguments are in registers:\n"
	" # RAX = syscall number\n"
	" # RDI = arg1, RSI = arg2, RDX = arg3\n"
	" # R10 = arg4, R8 = arg5, R9 = arg6\n"
	" \n"
	" # Save original args before overwriting\n"
	" movq %rdi, %rbx # Save arg1\n"
	" movq %rsi, %r11 # Save arg2\n"
	" movq %rdx, %r12 # Save arg3\n"
	" movq %r10, %r13 # Save arg4\n"
	" movq %r8, %r14 # Save arg5\n"
	" movq %r9, %r15 # Save arg6\n"
	" \n"
	" # Set up call to syscall_handler(syscall_num, arg1, arg2, arg3, arg4, arg5, arg6)\n"
	" movq %rax, %rdi # syscall_num -> RDI\n"
	" movq %rbx, %rsi # arg1 -> RSI\n"
	" movq %r11, %rdx # arg2 -> RDX\n"
	" movq %r12, %rcx # arg3 -> RCX\n"
	" movq %r13, %r8 # arg4 -> R8\n"
	" movq %r14, %r9 # arg5 -> R9\n"
	" pushq %r15 # arg6 -> stack\n"
	" \n"
	" call syscall_handler\n"
	" addq $8, %rsp # Clean up pushed arg6\n"
	" \n"
	" # Restore all registers except RAX (contains return value)\n"
	/*		
		Restore them
	*/
	" popq %r15\n"
	" popq %r14\n"
	" popq %r13\n"
	" popq %r12\n"
	" popq %r11\n"
	" popq %r10\n"
	" popq %r9\n"
	" popq %r8\n"
	" popq %rbp\n"
	" popq %rdi\n"
	" popq %rsi\n"
	" popq %rdx\n"
	" popq %rcx\n"
	" popq %rbx\n"
	" \n"
	" # Return to userspace\n"
	" iretq\n" // Back to the caller
);

/*

	INIT the SYSCALLS

*/
void init_syscalls (void) {
	extern void syscall_entry(void);
	/*
		Init these handles
	*/
	/*
		Print/debug handler
		printf.
	*/
	syscall_table[1].handler = printf_handler;
	syscall_table[1].name = "printf";
	syscall_table[1].arg_count = 6;

	/*
		Filesystem/Fat32
		open,
		close,
		read,
		write,
		lseek,
		mkdir,
		unlink
	*/
	syscall_table[2].handler = open_handler;
	syscall_table[2].name = "open";
	syscall_table[2].arg_count = 3;
	
	syscall_table[3].handler = close_handler;
	syscall_table[3].name = "close";
	syscall_table[3].arg_count = 1;
	
	syscall_table[4].handler = read_handler;
	syscall_table[4].name = "read";
	syscall_table[4].arg_count = 3;
	
	syscall_table[5].handler = write_handler;
	syscall_table[5].name = "write";
	syscall_table[5].arg_count = 3;
	
	syscall_table[6].handler = lseek_handler;
	syscall_table[6].name = "lseek";
	syscall_table[6].arg_count = 3;
	
	syscall_table[7].handler = mkdir_handler;
	syscall_table[7].name = "mkdir";
	syscall_table[7].arg_count = 2;
	
	syscall_table[8].handler = unlink_handler;
	syscall_table[8].name = "unlink";
	syscall_table[8].arg_count = 1;

	/*
		Memory management
		malloc,
		free,
		realloc,
		calloc
	*/
	syscall_table[9].handler = malloc_handler;
	syscall_table[9].name = "malloc";
	syscall_table[9].arg_count = 1;

	syscall_table[10].handler = free_handler;
	syscall_table[10].name = "free";
	syscall_table[10].arg_count = 1;

	syscall_table[11].handler = realloc_handler;
	syscall_table[11].name = "realloc";
	syscall_table[11].arg_count = 2;

	syscall_table[12].handler = calloc_handler;
	syscall_table[12].name = "calloc";
	syscall_table[12].arg_count = 2;
	/*
		Threading/multithreading INFORMATION
	*/
	syscall_table[13].handler = getpid_handler;
	syscall_table[13].name = "getpid";
	syscall_table[13].arg_count = 0;

	syscall_table[14].handler = gettid_handler;
	syscall_table[14].name = "gettid";
	syscall_table[14].arg_count = 0;

	syscall_table[15].handler = get_by_tid_handler;
    syscall_table[15].name = "get_by_tid";
    syscall_table[15].arg_count = 1;

    syscall_table[16].handler = thread_exists_handler;
    syscall_table[16].name = "thread_exists";
    syscall_table[16].arg_count = 1;

    syscall_table[17].handler = get_state_handler;
    syscall_table[17].name = "get_state";
    syscall_table[17].arg_count = 1;

	/*
		Multithreading
	*/
	syscall_table[18].handler = thread_create_handler;
    syscall_table[18].name    = "thread_create";
    syscall_table[18].arg_count = 5;

    syscall_table[19].handler = thread_execute_handler;
    syscall_table[19].name    = "thread_execute";
    syscall_table[19].arg_count = 1;

    syscall_table[20].handler = thread_exit_handler;
    syscall_table[20].name    = "thread_exit";
    syscall_table[20].arg_count = 0;

    syscall_table[21].handler = thread_terminate_handler;
    syscall_table[21].name    = "thread_terminate";
    syscall_table[21].arg_count = 1;

    syscall_table[28].handler = thread_block_handler;
    syscall_table[28].name = "thread_block";
    syscall_table[28].arg_count = 0;

    syscall_table[29].handler = thread_unblock_handler;
    syscall_table[29].name = "thread_unblock";
    syscall_table[29].arg_count = 0;
	/*
		proc/process
	*/
    syscall_table[22].handler = spawn_process_handler;
    syscall_table[22].name = "spawn_process";
    syscall_table[22].arg_count = 2;

    syscall_table[23].handler = execute_process_handler;
    syscall_table[23].name = "execute_process";
    syscall_table[23].arg_count = 1;

    syscall_table[24].handler = kill_process_handler;
    syscall_table[24].name = "kill_process";
    syscall_table[24].arg_count = 1;
	/*
		IPC pipes
	*/
	syscall_table[25].handler = makepipe_handler;
	syscall_table[25].name = "makepipe";
	syscall_table[25].arg_count = 2;

	syscall_table[26].handler = getpipe_handler;
	syscall_table[26].name = "getpipe";
	syscall_table[26].arg_count = 1;
	/*
		SYSTEM clock
	*/
	syscall_table[30].handler = time_now_ms_handler;
	syscall_table[30].name = "time_now_ms";
	syscall_table[30].arg_count = 0;

	syscall_table[31].handler = uptime_seconds_handler;
	syscall_table[31].name = "uptime_seconds";
	syscall_table[31].arg_count = 0;

	syscall_table[32].handler = uptime_minutes_handler;
	syscall_table[32].name = "uptime_minutes";
	syscall_table[32].arg_count = 0;

	syscall_table[33].handler = time_after_handler;
	syscall_table[33].name  = "time_after";
	syscall_table[33].arg_count = 2;

	syscall_table[34].handler = time_before_handler;
	syscall_table[34].name = "time_before";
	syscall_table[34].arg_count = 2;
	/*
		Set the IDT
	*/
	idt_set_entry(0x80, (uint64_t)syscall_entry, KERNEL_CODE_SELECTOR, 0xEE);
}
