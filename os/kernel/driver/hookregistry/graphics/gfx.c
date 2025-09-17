/*
	HEADER FILES
*/
#include "gfx.h" // The heart of this
/*
	Some layer
*/
/*

	Modes and buffer info

*/
void gfx_query_caps(uint64_t *caps_out, uint32_t *abi_version_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)caps_out;
    data.args[1] = (uint64_t)abi_version_out;
    HOOK_CALL("gfx_query_caps", &data);
}

void gfx_list_modes(void *mode_array_out, int max_modes, int *count_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)mode_array_out;
    data.args[1] = (uint64_t)max_modes;
    data.args[2] = (uint64_t)count_out;
    HOOK_CALL("gfx_list_modes", &data);
}

void gfx_get_mode(void *mode_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)mode_out;
    HOOK_CALL("gfx_get_mode", &data);
}

void gfx_set_mode(int mode_id) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)mode_id;
    HOOK_CALL("gfx_set_mode", &data);
}

void gfx_get_framebuffer_info(void **fb_base_out, int *pitch_out, int *mem_model_out, int *bpp_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)fb_base_out;
    data.args[1] = (uint64_t)pitch_out;
    data.args[2] = (uint64_t)mem_model_out;
    data.args[3] = (uint64_t)bpp_out;
    HOOK_CALL("gfx_get_framebuffer_info", &data);
}

/*

	Palette and gamma stuff

*/

void gfx_set_palette_entry(int index, uint8_t r, uint8_t g, uint8_t b) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)index;
    data.args[1] = (uint64_t)r;
    data.args[2] = (uint64_t)g;
    data.args[3] = (uint64_t)b;
    HOOK_CALL("gfx_set_palette_entry", &data);
}

void gfx_get_palette_entry(int index, uint8_t *r_out, uint8_t *g_out, uint8_t *b_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)index;
    data.args[1] = (uint64_t)r_out;
    data.args[2] = (uint64_t)g_out;
    data.args[3] = (uint64_t)b_out;
    HOOK_CALL("gfx_get_palette_entry", &data);
}

void gfx_set_palette(void *rgb_array, int count) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)rgb_array;
    data.args[1] = (uint64_t)count;
    HOOK_CALL("gfx_set_palette", &data);
}

void gfx_set_gamma(void *r_table, void *g_table, void *b_table, int count) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)r_table;
    data.args[1] = (uint64_t)g_table;
    data.args[2] = (uint64_t)b_table;
    data.args[3] = (uint64_t)count;
    HOOK_CALL("gfx_set_gamma", &data);
}

void gfx_set_brightness(int percent) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)percent;
    HOOK_CALL("gfx_set_brightness", &data);
}

void gfx_set_contrast(int percent) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)percent;
    HOOK_CALL("gfx_set_contrast", &data);
}

/*

	Cursor (hardware one)

*/

void gfx_show_cursor(int show) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)show;
    HOOK_CALL("gfx_show_cursor", &data);
}

void gfx_set_cursor_bitmap(void *bitmap, int w, int h) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)bitmap;
    data.args[1] = (uint64_t)w;
    data.args[2] = (uint64_t)h;
    HOOK_CALL("gfx_set_cursor_bitmap", &data);
}

void gfx_set_cursor_hotspot(int hx, int hy) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)hx;
    data.args[1] = (uint64_t)hy;
    HOOK_CALL("gfx_set_cursor_hotspot", &data);
}

void gfx_set_cursor_pos(int x, int y) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x;
    data.args[1] = (uint64_t)y;
    HOOK_CALL("gfx_set_cursor_pos", &data);
}

/*

	Drawing

*/

void gfx_clear(uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)color;
    HOOK_CALL("gfx_clear", &data);
}

void gfx_draw_pixel(int x, int y, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x;
    data.args[1] = (uint64_t)y;
    data.args[2] = (uint64_t)color;
    HOOK_CALL("gfx_draw_pixel", &data);
}

void gfx_draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x0;
    data.args[1] = (uint64_t)y0;
    data.args[2] = (uint64_t)x1;
    data.args[3] = (uint64_t)y1;
    data.args[4] = (uint64_t)color;
    HOOK_CALL("gfx_draw_line", &data);
}

void gfx_draw_rect(int x, int y, int w, int h, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x;
    data.args[1] = (uint64_t)y;
    data.args[2] = (uint64_t)w;
    data.args[3] = (uint64_t)h;
    data.args[4] = (uint64_t)color;
    HOOK_CALL("gfx_draw_rect", &data);
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x;
    data.args[1] = (uint64_t)y;
    data.args[2] = (uint64_t)w;
    data.args[3] = (uint64_t)h;
    data.args[4] = (uint64_t)color;
    HOOK_CALL("gfx_fill_rect", &data);
}

void gfx_draw_circle(int cx, int cy, int r, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)cx;
    data.args[1] = (uint64_t)cy;
    data.args[2] = (uint64_t)r;
    data.args[3] = (uint64_t)color;
    HOOK_CALL("gfx_draw_circle", &data);
}

void gfx_fill_circle(int cx, int cy, int r, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)cx;
    data.args[1] = (uint64_t)cy;
    data.args[2] = (uint64_t)r;
    data.args[3] = (uint64_t)color;
    HOOK_CALL("gfx_fill_circle", &data);
}

void gfx_draw_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x0; data.args[1] = (uint64_t)y0;
    data.args[2] = (uint64_t)x1; data.args[3] = (uint64_t)y1;
    data.args[4] = (uint64_t)x2; data.args[5] = (uint64_t)y2;
    data.args[6] = (uint64_t)color;
    HOOK_CALL("gfx_draw_triangle", &data);
}

void gfx_fill_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x0; data.args[1] = (uint64_t)y0;
    data.args[2] = (uint64_t)x1; data.args[3] = (uint64_t)y1;
    data.args[4] = (uint64_t)x2; data.args[5] = (uint64_t)y2;
    data.args[6] = (uint64_t)color;
    HOOK_CALL("gfx_fill_triangle", &data);
}

void gfx_draw_polygon(void *points_xy16, int count, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)points_xy16;
    data.args[1] = (uint64_t)count;
    data.args[2] = (uint64_t)color;
    HOOK_CALL("gfx_draw_polygon", &data);
}

void gfx_fill_polygon(void *points_xy16, int count, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)points_xy16;
    data.args[1] = (uint64_t)count;
    data.args[2] = (uint64_t)color;
    HOOK_CALL("gfx_fill_polygon", &data);
}

/*

	Bliting

*/

void gfx_create_surface(int w, int h, int fmt, void **handle_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)w;
    data.args[1] = (uint64_t)h;
    data.args[2] = (uint64_t)fmt;
    data.args[3] = (uint64_t)handle_out;
    HOOK_CALL("gfx_create_surface", &data);
}

void gfx_destroy_surface(void *handle) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)handle;
    HOOK_CALL("gfx_destroy_surface", &data);
}

void gfx_lock_surface(void *handle, void **pixels_out, int *pitch_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)handle;
    data.args[1] = (uint64_t)pixels_out;
    data.args[2] = (uint64_t)pitch_out;
    HOOK_CALL("gfx_lock_surface", &data);
}

void gfx_unlock_surface(void *handle) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)handle;
    HOOK_CALL("gfx_unlock_surface", &data);
}

void gfx_surface_fill(void *dst, int x, int y, int w, int h, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)dst;
    data.args[1] = (uint64_t)x;
    data.args[2] = (uint64_t)y;
    data.args[3] = (uint64_t)w;
    data.args[4] = (uint64_t)h;
    data.args[5] = (uint64_t)color;
    HOOK_CALL("gfx_surface_fill", &data);
}

void gfx_blit(void *src_pixels, int src_pitch, int sx, int sy, int w, int h, int dx, int dy, int fmt) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)src_pixels;
    data.args[1] = (uint64_t)src_pitch;
    data.args[2] = (uint64_t)sx;
    data.args[3] = (uint64_t)sy;
    data.args[4] = (uint64_t)w;
    data.args[5] = (uint64_t)h;
    data.args[6] = (uint64_t)dx;
    data.args[7] = (uint64_t)dy;
    data.args[8] = (uint64_t)fmt;
    HOOK_CALL("gfx_blit", &data);
}

void gfx_surface_blit(void *src_surf, int sx, int sy, int w, int h, void *dst_surf, int dx, int dy) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)src_surf;
    data.args[1] = (uint64_t)sx;
    data.args[2] = (uint64_t)sy;
    data.args[3] = (uint64_t)w;
    data.args[4] = (uint64_t)h;
    data.args[5] = (uint64_t)dst_surf;
    data.args[6] = (uint64_t)dx;
    data.args[7] = (uint64_t)dy;
    HOOK_CALL("gfx_surface_blit", &data);
}

void gfx_blit_colorkey(void *src_pixels, int src_pitch, int sx,int sy,int w,int h,int dx,int dy,uint32_t key, int fmt) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)src_pixels; data.args[1]=(uint64_t)src_pitch;
    data.args[2]=(uint64_t)sx; data.args[3]=(uint64_t)sy;
    data.args[4]=(uint64_t)w;  data.args[5]=(uint64_t)h;
    data.args[6]=(uint64_t)dx; data.args[7]=(uint64_t)dy;
    data.args[8]=(uint64_t)key; data.args[9]=(uint64_t)fmt;
    HOOK_CALL("gfx_blit_colorkey", &data);
}

void gfx_blit_alpha(void *src_pixels, int src_pitch, int sx,int sy,int w,int h,int dx,int dy,uint8_t alpha, int fmt) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)src_pixels; data.args[1]=(uint64_t)src_pitch;
    data.args[2]=(uint64_t)sx; data.args[3]=(uint64_t)sy;
    data.args[4]=(uint64_t)w;  data.args[5]=(uint64_t)h;
    data.args[6]=(uint64_t)dx; data.args[7]=(uint64_t)dy;
    data.args[8]=(uint64_t)alpha; data.args[9]=(uint64_t)fmt;
    HOOK_CALL("gfx_blit_alpha", &data);
}

void gfx_stretch_blit(void *src_pixels, int src_pitch, int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh,int fmt) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)src_pixels; data.args[1]=(uint64_t)src_pitch;
    data.args[2]=(uint64_t)sx; data.args[3]=(uint64_t)sy;
    data.args[4]=(uint64_t)sw; data.args[5]=(uint64_t)sh;
    data.args[6]=(uint64_t)dx; data.args[7]=(uint64_t)dy;
    data.args[8]=(uint64_t)dw; data.args[9]=(uint64_t)dh;
    data.args[10]=(uint64_t)fmt;
    HOOK_CALL("gfx_stretch_blit", &data);
}

void gfx_roto_blit(void *src_pixels, int src_pitch, int sx,int sy,int w,int h,int cx,int cy,float angle,int dx,int dy,int fmt) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)src_pixels; data.args[1]=(uint64_t)src_pitch;
    data.args[2]=(uint64_t)sx; data.args[3]=(uint64_t)sy;
    data.args[4]=(uint64_t)w;  data.args[5]=(uint64_t)h;
    data.args[6]=(uint64_t)cx; data.args[7]=(uint64_t)cy;
    data.args[8]=*(uint64_t*)&angle; // bit-cast float
    data.args[9]=(uint64_t)dx; data.args[10]=(uint64_t)dy;
    data.args[11]=(uint64_t)fmt;
    HOOK_CALL("gfx_roto_blit", &data);
}

/*

	Some extras (too lazy to name em all here)

*/

void gfx_set_clip_rect(int x, int y, int w, int h) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)x; data.args[1]=(uint64_t)y;
    data.args[2]=(uint64_t)w; data.args[3]=(uint64_t)h;
    HOOK_CALL("gfx_set_clip_rect", &data);
}

void gfx_reset_clip(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_reset_clip", &data);
}

void gfx_set_blend_mode(int mode) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)mode;
    HOOK_CALL("gfx_set_blend_mode", &data);
}

void gfx_set_line_width(int px) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)px;
    HOOK_CALL("gfx_set_line_width", &data);
}

void gfx_set_antialias(int enabled) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)enabled;
    HOOK_CALL("gfx_set_antialias", &data);
}

void gfx_swap_buffers(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_swap_buffers", &data);
}

void gfx_vsync_wait(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_vsync_wait", &data);
}
void gfx_present(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_present", &data);
}

void gfx_begin_batch(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_begin_batch", &data);
}

void gfx_end_batch(void) {
    struct hook_data data = {0};
    HOOK_CALL("gfx_end_batch", &data);
}

void gfx_submit_command_buffer(void *cmds, int bytes) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)cmds;
    data.args[1]=(uint64_t)bytes;
    HOOK_CALL("gfx_submit_command_buffer", &data);
}

void gfx_fence_create(uint64_t *fence_id_out) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)fence_id_out;
    HOOK_CALL("gfx_fence_create", &data);
}

void gfx_fence_wait(uint64_t fence_id, int timeout_ms) {
    struct hook_data data = {0};
    data.args[0]=(uint64_t)fence_id;
    data.args[1]=(uint64_t)timeout_ms;
    HOOK_CALL("gfx_fence_wait", &data);
}