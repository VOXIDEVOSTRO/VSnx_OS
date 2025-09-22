#ifndef MOUSE_H
#define MOUSE_H
/*
	Head
*/
#include "../../hook.h"
/*
	Prototypes
*/
void mouse_init(void);
void mouse_shutdown(void);
void mouse_get_position(int32_t *x_out, int32_t *y_out);
void mouse_set_position(int32_t x, int32_t y);
void mouse_get_buttons(uint32_t *buttons_out);
void mouse_poll_event(int32_t *dx_out, int32_t *dy_out, int32_t *wheel_out, uint32_t *buttons_out);
void mouse_set_cursor_visible(uint32_t visible);
void mouse_set_cursor_shape(uint64_t cursor_handle);
#endif