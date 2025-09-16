/*
	Header files
*/
#include <stdbool.h>
#include "hookcall.h"
#include "../hook.h"
#include "../../threading/thread.h"
#include "../../elf/process.h"
#include "../../fat32/gristle.h" // For fat32
#include "../../hal/mem/u_mem/u_mem.h" // For memory

int64_t register_hook_handler(uint64_t name_ptr, uint64_t func_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    const char* name = (const char*)name_ptr;
    hook_func_t func = (hook_func_t)func_ptr;
    register_hook(name, func);
    return 0;
}

/*




	GFX Handlers




*/

int64_t gfx_set_mode_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_mode");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_create_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_create_context");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_display_start_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_display_start");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_fill_rect_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_fill_rect");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_capture_frame_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_capture_frame");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_blit_surface_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_blit_surface");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_gamma_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_gamma");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_get_palette_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_get_palette");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_brightness_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_brightness");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_draw_pixel_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_draw_pixel");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_cursor_position_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_cursor_position");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_context");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_scroll_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_scroll");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_map_framebuffer_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_map_framebuffer");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_enable_cursor_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_enable_cursor");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_get_mode_info_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_get_mode_info");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_palette_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_palette");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_destroy_context_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_destroy_context");
    if (func) func((void*)data_ptr);
    return 0;
}

int64_t gfx_set_cursor_bitmap_handler(uint64_t data_ptr, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    hook_func_t func = get_hook("gfx_set_cursor_bitmap");
    if (func) func((void*)data_ptr);
    return 0;
}

/*






	Pagaing Handlers





	Just to make it we dont give too much freedom to the driver and make it simpler to map
	also this is auto generated via the python script
*/

int64_t paging_map_page_handler(uint64_t vaddr, uint64_t paddr, uint64_t flags, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    return paging_map_page(vaddr, paddr, (uint32_t)flags);
}

int64_t paging_unmap_page_handler(uint64_t vaddr, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    paging_unmap_page(vaddr);
    return 0;
}

int64_t paging_get_physical_handler(uint64_t vaddr, uint64_t out_ptr, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    *(uint64_t*)out_ptr = paging_get_physical(vaddr);
    return 0;
}

int64_t paging_print_info_handler(uint64_t unused1, uint64_t unused2, uint64_t unused3, uint64_t unused4, uint64_t unused5, uint64_t unused6) {
    paging_print_info();
    return 0;
}
