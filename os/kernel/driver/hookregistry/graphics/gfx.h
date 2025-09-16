#ifndef GFX_H
#define GFX_H
/*
	Head
*/
#include "../../hook.h"
/*
	prototypes
	very simple for normal or THE most extremeist graphic drivers
*/
void gfx_set_mode(int width, int height, int bpp);
void gfx_get_mode_info(int mode_id);
void gfx_map_framebuffer(void *addr, size_t size);
void gfx_draw_pixel(int x, int y, uint32_t color);
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_blit_surface(void *src, void *dst, int sx, int sy, int dx, int dy, int w, int h, int flags, int layer);
void gfx_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void gfx_get_palette(uint8_t index);
void gfx_scroll(int dx, int dy);
void gfx_set_display_start(int x, int y);
void gfx_create_context(int flags);
void gfx_set_context(int ctx_id);
void gfx_destroy_context(int ctx_id);
void gfx_capture_frame(void *buffer, size_t size);
void gfx_set_brightness(int level);
void gfx_set_gamma(float r, float g, float b);
void gfx_enable_cursor(int visible);
void gfx_set_cursor_position(int x, int y);
void gfx_set_cursor_bitmap(void *bitmap, int w, int h);
#endif