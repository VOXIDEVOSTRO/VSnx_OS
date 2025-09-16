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
		GFX
	*/
	hookcall_table[2].handler   = gfx_set_mode_handler;
	hookcall_table[2].name      = "gfx_set_mode";
	hookcall_table[2].arg_count = 1;	

	hookcall_table[3].handler   = gfx_create_context_handler;
	hookcall_table[3].name      = "gfx_create_context";
	hookcall_table[3].arg_count = 1;	

	hookcall_table[4].handler   = gfx_set_display_start_handler;
	hookcall_table[4].name      = "gfx_set_display_start";
	hookcall_table[4].arg_count = 1;	

	hookcall_table[5].handler   = gfx_fill_rect_handler;
	hookcall_table[5].name      = "gfx_fill_rect";
	hookcall_table[5].arg_count = 1;	

	hookcall_table[6].handler   = gfx_capture_frame_handler;
	hookcall_table[6].name      = "gfx_capture_frame";
	hookcall_table[6].arg_count = 1;	

	hookcall_table[7].handler   = gfx_blit_surface_handler;
	hookcall_table[7].name      = "gfx_blit_surface";
	hookcall_table[7].arg_count = 1;	

	hookcall_table[8].handler   = gfx_set_gamma_handler;
	hookcall_table[8].name      = "gfx_set_gamma";
	hookcall_table[8].arg_count = 1;	

	hookcall_table[9].handler   = gfx_get_palette_handler;
	hookcall_table[9].name      = "gfx_get_palette";
	hookcall_table[9].arg_count = 1;	

	hookcall_table[10].handler  = gfx_set_brightness_handler;
	hookcall_table[10].name     = "gfx_set_brightness";
	hookcall_table[10].arg_count = 1;	

	hookcall_table[11].handler  = gfx_draw_pixel_handler;
	hookcall_table[11].name     = "gfx_draw_pixel";
	hookcall_table[11].arg_count = 1;	

	hookcall_table[12].handler  = gfx_set_cursor_position_handler;
	hookcall_table[12].name     = "gfx_set_cursor_position";
	hookcall_table[12].arg_count = 1;	

	hookcall_table[13].handler  = gfx_set_context_handler;
	hookcall_table[13].name     = "gfx_set_context";
	hookcall_table[13].arg_count = 1;	

	hookcall_table[14].handler  = gfx_scroll_handler;
	hookcall_table[14].name     = "gfx_scroll";
	hookcall_table[14].arg_count = 1;	

	hookcall_table[15].handler  = gfx_map_framebuffer_handler;
	hookcall_table[15].name     = "gfx_map_framebuffer";
	hookcall_table[15].arg_count = 1;	

	hookcall_table[16].handler  = gfx_enable_cursor_handler;
	hookcall_table[16].name     = "gfx_enable_cursor";
	hookcall_table[16].arg_count = 1;	

	hookcall_table[17].handler  = gfx_get_mode_info_handler;
	hookcall_table[17].name     = "gfx_get_mode_info";
	hookcall_table[17].arg_count = 1;	

	hookcall_table[18].handler  = gfx_set_palette_handler;
	hookcall_table[18].name     = "gfx_set_palette";
	hookcall_table[18].arg_count = 1;	

	hookcall_table[19].handler  = gfx_destroy_context_handler;
	hookcall_table[19].name     = "gfx_destroy_context";
	hookcall_table[19].arg_count = 1;	

	hookcall_table[20].handler  = gfx_set_cursor_bitmap_handler;
	hookcall_table[20].name     = "gfx_set_cursor_bitmap";
	hookcall_table[20].arg_count = 1;
	/*
		MEM
	*/
	hookcall_table[21].handler = paging_map_page_handler;
	hookcall_table[21].name = "paging_map_page";
	hookcall_table[21].arg_count = 3;

	hookcall_table[22].handler = paging_unmap_page_handler;
	hookcall_table[22].name = "paging_unmap_page";
	hookcall_table[22].arg_count = 1;

	hookcall_table[23].handler = paging_get_physical_handler;
	hookcall_table[23].name = "paging_get_physical";
	hookcall_table[23].arg_count = 2;

	hookcall_table[24].handler = paging_print_info_handler;
	hookcall_table[24].name = "paging_print_info";
	hookcall_table[24].arg_count = 0;
	/*
		HOOKS
	*/
	hookcall_table[1].handler = register_hook_handler;
	hookcall_table[1].name = "register_hook";
	hookcall_table[1].arg_count = 2;
	/*
		Set the IDT
	*/
	idt_set_entry(0x81, (uint64_t)hookcalls_entry, KERNEL_CODE_SELECTOR, 0xEE);
}
