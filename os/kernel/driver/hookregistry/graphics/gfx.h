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
void gfx_query_caps(uint64_t *caps_out, uint32_t *abi_version_out);
void gfx_list_modes(void *mode_array_out, int max_modes, int *count_out);
void gfx_get_mode(void *mode_out);
void gfx_set_mode(int mode_id);
void gfx_get_framebuffer_info(void **fb_base_out, int *pitch_out, int *mem_model_out, int *bpp_out);
void gfx_set_palette_entry(int index, uint8_t r, uint8_t g, uint8_t b);
void gfx_get_palette_entry(int index, uint8_t *r_out, uint8_t *g_out, uint8_t *b_out);
void gfx_set_palette(void *rgb_array, int count);
void gfx_set_gamma(void *r_table, void *g_table, void *b_table, int count);
void gfx_set_brightness(int percent);
void gfx_set_contrast(int percent);
void gfx_show_cursor(int show);
void gfx_set_cursor_bitmap(void *bitmap, int w, int h);
void gfx_set_cursor_hotspot(int hx, int hy);
void gfx_set_cursor_pos(int x, int y);
void gfx_clear(uint32_t color);
void gfx_draw_pixel(int x, int y, uint32_t color);
void gfx_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void gfx_draw_rect(int x, int y, int w, int h, uint32_t color);
void gfx_fill_rect(int x, int y, int w, int h, uint32_t color);
void gfx_draw_circle(int cx, int cy, int r, uint32_t color);
void gfx_fill_circle(int cx, int cy, int r, uint32_t color);
void gfx_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void gfx_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void gfx_draw_polygon(void *points_xy16, int count, uint32_t color);
void gfx_fill_polygon(void *points_xy16, int count, uint32_t color);
void gfx_create_surface(int w, int h, int fmt, void **handle_out);
void gfx_destroy_surface(void *handle);
void gfx_lock_surface(void *handle, void **pixels_out, int *pitch_out);
void gfx_unlock_surface(void *handle);
void gfx_surface_fill(void *dst, int x, int y, int w, int h, uint32_t color);
void gfx_blit(void *src_pixels, int src_pitch, int sx, int sy, int w, int h, int dx, int dy, int fmt);
void gfx_surface_blit(void *src_surf, int sx, int sy, int w, int h, void *dst_surf, int dx, int dy);
void gfx_blit_colorkey(void *src_pixels, int src_pitch, int sx, int sy, int w, int h, int dx, int dy, uint32_t key, int fmt);
void gfx_blit_alpha(void *src_pixels, int src_pitch, int sx, int sy, int w, int h, int dx, int dy, uint8_t alpha, int fmt);
void gfx_stretch_blit(void *src_pixels, int src_pitch, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int fmt);
void gfx_roto_blit(void *src_pixels, int src_pitch, int sx, int sy, int w, int h, int cx, int cy, float angle, int dx, int dy, int fmt);
void gfx_set_clip_rect(int x, int y, int w, int h);
void gfx_reset_clip(void);
void gfx_set_blend_mode(int mode);
void gfx_set_line_width(int px);
void gfx_set_antialias(int enabled);
void gfx_swap_buffers(void);
void gfx_vsync_wait(void);
void gfx_present(void);
void gfx_begin_batch(void);
void gfx_end_batch(void);
void gfx_submit_command_buffer(void *cmds, int bytes);
void gfx_fence_create(uint64_t *fence_id_out);
void gfx_fence_wait(uint64_t fence_id, int timeout_ms);
#endif