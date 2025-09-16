/*
	HEADER FILES
*/
#include "gfx.h" // The heart of this
/*
	Some layer
*/
void gfx_set_mode(int width, int height, int bpp) {
    struct hook_data data = {0};
    data.args[0] = width;
    data.args[1] = height;
    data.args[2] = bpp;
    HOOK_CALL("gfx_set_mode", &data);
}

void gfx_get_mode_info(int mode_id) {
    struct hook_data data = {0};
    data.args[0] = mode_id;
    HOOK_CALL("gfx_get_mode_info", &data);
}

void gfx_map_framebuffer(void *addr, size_t size) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)addr;
    data.args[1] = size;
    HOOK_CALL("gfx_map_framebuffer", &data);
}

void gfx_draw_pixel(int x, int y, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = x;
    data.args[1] = y;
    data.args[2] = color;
    HOOK_CALL("gfx_draw_pixel", &data);
}

void gfx_fill_rect(int x, int y, int w, int h, uint32_t color) {
    struct hook_data data = {0};
    data.args[0] = x;
    data.args[1] = y;
    data.args[2] = w;
    data.args[3] = h;
    data.args[4] = color;
    HOOK_CALL("gfx_fill_rect", &data);
}

void gfx_blit_surface(void *src, void *dst, int sx, int sy, int dx, int dy, int w, int h, int flags, int layer) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)src;
    data.args[1] = (uint64_t)dst;
    data.args[2] = sx;
    data.args[3] = sy;
    data.args[4] = dx;
    data.args[5] = dy;
    data.args[6] = w;
    data.args[7] = h;
    data.args[8] = flags;
    data.args[9] = layer;
    HOOK_CALL("gfx_blit_surface", &data);
}

void gfx_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    struct hook_data data = {0};
    data.args[0] = index;
    data.args[1] = r;
    data.args[2] = g;
    data.args[3] = b;
    HOOK_CALL("gfx_set_palette", &data);
}

void gfx_get_palette(uint8_t index) {
    struct hook_data data = {0};
    data.args[0] = index;
    HOOK_CALL("gfx_get_palette", &data);
}

void gfx_scroll(int dx, int dy) {
    struct hook_data data = {0};
    data.args[0] = dx;
    data.args[1] = dy;
    HOOK_CALL("gfx_scroll", &data);
}

void gfx_set_display_start(int x, int y) {
    struct hook_data data = {0};
    data.args[0] = x;
    data.args[1] = y;
    HOOK_CALL("gfx_set_display_start", &data);
}

// Context management
void gfx_create_context(int flags) {
    struct hook_data data = {0};
    data.args[0] = flags;
    HOOK_CALL("gfx_create_context", &data);
}

void gfx_set_context(int ctx_id) {
    struct hook_data data = {0};
    data.args[0] = ctx_id;
    HOOK_CALL("gfx_set_context", &data);
}

void gfx_destroy_context(int ctx_id) {
    struct hook_data data = {0};
    data.args[0] = ctx_id;
    HOOK_CALL("gfx_destroy_context", &data);
}

void gfx_capture_frame(void *buffer, size_t size) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)buffer;
    data.args[1] = size;
    HOOK_CALL("gfx_capture_frame", &data);
}

void gfx_set_brightness(int level) {
    struct hook_data data = {0};
    data.args[0] = level;
    HOOK_CALL("gfx_set_brightness", &data);
}

void gfx_set_gamma(float r, float g, float b) {
    struct hook_data data = {0};
    data.args[0] = *(uint64_t *)&r;
    data.args[1] = *(uint64_t *)&g;
    data.args[2] = *(uint64_t *)&b;
    HOOK_CALL("gfx_set_gamma", &data);
}

void gfx_enable_cursor(int visible) {
    struct hook_data data = {0};
    data.args[0] = visible;
    HOOK_CALL("gfx_enable_cursor", &data);
}

void gfx_set_cursor_position(int x, int y) {
    struct hook_data data = {0};
    data.args[0] = x;
    data.args[1] = y;
    HOOK_CALL("gfx_set_cursor_position", &data);
}

void gfx_set_cursor_bitmap(void *bitmap, int w, int h) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)bitmap;
    data.args[1] = w;
    data.args[2] = h;
    HOOK_CALL("gfx_set_cursor_bitmap", &data);
}
