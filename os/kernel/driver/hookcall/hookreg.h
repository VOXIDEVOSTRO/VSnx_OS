#ifndef HOOKREG_H
#define HOOKREG_H
/*
	header files
*/
#include "hookcall.h"
#include "../hook.h"
/*
	Boring registeration
*/
static inline int reg_hook(const char* name, hook_func_t func) {
	return hookcall(1/*register hook*/, (uint64_t)name/*The name*/, (uint64_t)func/*The pointer the driver will pass*/, /*rest are NULL just ignore*/0, 0, 0, 0);
}
/*



	GFX


	// why the hell the GFX so small
*/
static inline void gfx_set_mode(void* data) {
    hookcall(2, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_create_context(void* data) {
    hookcall(3, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_display_start(void* data) {
    hookcall(4, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_fill_rect(void* data) {
    hookcall(5, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_capture_frame(void* data) {
    hookcall(6, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_blit_surface(void* data) {
    hookcall(7, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_gamma(void* data) {
    hookcall(8, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_get_palette(void* data) {
    hookcall(9, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_brightness(void* data) {
    hookcall(10, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_draw_pixel(void* data) {
    hookcall(11, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_cursor_position(void* data) {
    hookcall(12, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_context(void* data) {
    hookcall(13, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_scroll(void* data) {
    hookcall(14, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_map_framebuffer(void* data) {
    hookcall(15, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_enable_cursor(void* data) {
    hookcall(16, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_get_mode_info(void* data) {
    hookcall(17, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_palette(void* data) {
    hookcall(18, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_destroy_context(void* data) {
    hookcall(19, (uint64_t)data, 0, 0, 0, 0, 0);
}

static inline void gfx_set_cursor_bitmap(void* data) {
    hookcall(20, (uint64_t)data, 0, 0, 0, 0, 0);
}

/*



	Pagaing for mapping and stuff



*/

static inline int map_page(uint64_t vaddr, uint64_t paddr, uint32_t flags) {
    return hookcall(21, vaddr, paddr, flags, 0, 0, 0);
}

static inline void unmap_page(uint64_t vaddr) {
    hookcall(22, vaddr, 0, 0, 0, 0, 0);
}

static inline uint64_t get_physical(uint64_t vaddr) {
    uint64_t out_phys = 0;
    hookcall(23, vaddr, (uint64_t)&out_phys, 0, 0, 0, 0);
    return out_phys;
}

#endif
