#ifndef HOOKSET_H
#define HOOKSET_H
/*
	header files
*/
#include "hookcall.h"
/*
	Prototypes
	I will use python script or cproto for now because too repeitative
*/
int64_t register_hook_handler(uint64_t type, uint64_t func, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
int64_t gfx_set_mode_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_create_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_display_start_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_fill_rect_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_capture_frame_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_blit_surface_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_gamma_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_get_palette_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_brightness_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_draw_pixel_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_cursor_position_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_scroll_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_map_framebuffer_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_enable_cursor_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_get_mode_info_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_palette_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_destroy_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t gfx_set_cursor_bitmap_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);
int64_t paging_map_page_handler(uint64_t vaddr, uint64_t paddr, uint64_t flags, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_unmap_page_handler(uint64_t vaddr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_get_physical_handler(uint64_t vaddr, uint64_t out_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
int64_t paging_print_info_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6);
#endif
