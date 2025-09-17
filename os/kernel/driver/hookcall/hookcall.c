/*

	Header files

*/
#include "hookcall.h"
#include "hookset.h"
/*

	MAIN HANDLER

*/
void hookcall_handler(uint64_t hookcall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
	if (hookcall_num >= MAX_HOOKCALLS || hookcall_table[hookcall_num].handler == NULL) {
 		__asm__ volatile("movq $-1, %%rax" : : : "rax");
 		return; // dah
 	}
 	int64_t result = hookcall_table[hookcall_num].handler(arg1, arg2, arg3, arg4, arg5, arg6);
 	__asm__ volatile("movq %0, %%rax" : : "r"(result) : "rax"); // returns as usual
}
/*

	ASSEMBLY BLOCK For handling the hookcall interrupt which is 0x81

*/
__asm__(
	".global hookcalls_entry\n"
	"hookcalls_entry:\n"
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
	" movq %rdi, %rbx # Save arg1\n"
	" movq %rsi, %r11 # Save arg2\n"
	" movq %rdx, %r12 # Save arg3\n"
	" movq %r10, %r13 # Save arg4\n"
	" movq %r8, %r14 # Save arg5\n"
	" movq %r9, %r15 # Save arg6\n"
	" \n"
	" movq %rax, %rdi # syscall_num -> RDI\n"
	" movq %rbx, %rsi # arg1 -> RSI\n"
	" movq %r11, %rdx # arg2 -> RDX\n"
	" movq %r12, %rcx # arg3 -> RCX\n"
	" movq %r13, %r8 # arg4 -> R8\n"
	" movq %r14, %r9 # arg5 -> R9\n"
	" pushq %r15 # arg6 -> stack\n"
	" \n"
	" call hookcall_handler\n"
	" addq $8, %rsp # Clean up pushed arg6\n"
	" \n"
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
	" iretq\n" // Round back
);

/*

	INIT the SYSCALLS

*/
void init_hookcalls (void) {
	extern void hookcalls_entry(void);
	/*
		Init these handles
	*/
	/*
		MEM
	*/
	hookcall_table[7].handler = paging_map_page_handler;
	hookcall_table[7].name = "paging_map_page";
	hookcall_table[7].arg_count = 3;

	hookcall_table[6].handler = paging_unmap_page_handler;
	hookcall_table[6].name = "paging_unmap_page";
	hookcall_table[6].arg_count = 1;

	hookcall_table[5].handler = paging_get_physical_handler;
	hookcall_table[5].name = "paging_get_physical";
	hookcall_table[5].arg_count = 2;

	hookcall_table[4].handler = paging_print_info_handler;
	hookcall_table[4].name = "paging_print_info";
	hookcall_table[4].arg_count = 0;
	/*
		HOOKS
	*/
	hookcall_table[3].handler = get_hook_handler;
	hookcall_table[3].name = "get_hook";
	hookcall_table[3].arg_count = 1;

	hookcall_table[2].handler = call_hook_handler;
	hookcall_table[2].name = "call_hook";
	hookcall_table[2].arg_count = 2;

	hookcall_table[1].handler = register_hook_handler;
	hookcall_table[1].name = "register_hook";
	hookcall_table[1].arg_count = 2;
	/*
		Set the IDT
	*/
	idt_set_entry(0x81, (uint64_t)hookcalls_entry, KERNEL_CODE_SELECTOR, 0xEE);
}
