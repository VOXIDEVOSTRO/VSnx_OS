/*
	Headerz
*/
#include "mouse.h"
/*
	Main hookling wrappers
	Mostly simple stuff
*/
void mouse_init(void) 
{
    struct hook_data data = {0};
    HOOK_CALL("mouse_init", &data);
}
void mouse_shutdown(void) 
{
    struct hook_data data = {0};
    HOOK_CALL("mouse_shutdown", &data);
}
void mouse_get_position(int32_t *x_out, int32_t *y_out) 
{
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x_out;
    data.args[1] = (uint64_t)y_out;
    HOOK_CALL("mouse_get_position", &data);
}
void mouse_set_position(int32_t x, int32_t y) 
{
    struct hook_data data = {0};
    data.args[0] = (uint64_t)x;
    data.args[1] = (uint64_t)y;
    HOOK_CALL("mouse_set_position", &data);
}
void mouse_get_buttons(uint32_t *buttons_out) 
{
    struct hook_data data = {0};
    data.args[0] = (uint64_t)buttons_out;
    HOOK_CALL("mouse_get_buttons", &data);
}
void mouse_poll_event(int32_t *dx_out, int32_t *dy_out, int32_t *wheel_out, uint32_t *buttons_out) 
{
    struct hook_data data = {0};
    data.args[0] = (uint64_t)dx_out;
    data.args[1] = (uint64_t)dy_out;
    data.args[2] = (uint64_t)wheel_out;
    data.args[3] = (uint64_t)buttons_out;
    HOOK_CALL("mouse_poll_event", &data);
}
void mouse_set_cursor_visible(uint32_t visible) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)visible;
    HOOK_CALL("mouse_set_cursor_visible", &data);
}
void mouse_set_cursor_shape(uint64_t cursor_handle) {
    struct hook_data data = {0};
    data.args[0] = cursor_handle;
    HOOK_CALL("mouse_set_cursor_shape", &data);
}