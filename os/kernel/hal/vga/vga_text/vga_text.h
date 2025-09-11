#ifndef VGA_TEXT_H
#define VGA_TEXT_H
/*
	Header files
*/
#include <stdint.h>
#include "../../io/io_ports.h"
#include "../../serial_com/serial.h" // DEBUG: Just for normal debug output
/*
	Macros
*/
#define VGA_TEXT_WIDTH 80
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_STATUS_REG 0x3DA // status
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_MEMORY 0xB8000 // standard VGA addr
/*
	Some (all) 15 colors
*/
typedef enum {
    VGA_BLACK = 0, VGA_BLUE = 1, VGA_GREEN = 2, VGA_CYAN = 3,
    VGA_RED = 4, VGA_MAGENTA = 5, VGA_BROWN = 6, VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8, VGA_LIGHT_BLUE = 9, VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11, VGA_LIGHT_RED = 12, VGA_LIGHT_MAGENTA = 13,
    VGA_LIGHT_BROWN = 14, VGA_WHITE = 15
} vga_color_t;
/*
	Globals
*/
extern volatile uint16_t* vga_buffer;
extern int cursor_x;
extern int cursor_y;
extern uint8_t current_color;
/*
	prototypes
*/
/*
	vga_dec_text.c
*/
int vga_dec_text(void);
/*
	vga_text.c
*/
uint8_t vga_make_color(vga_color_t fg, vga_color_t bg);
uint16_t vga_make_entry(char c, uint8_t color);
void vga_update_cursor(void/*Take no Args*/);
void vga_scroll(void);
int vga_text_init(void);
void vga_text_clear(void);
void vga_text_set_color(vga_color_t fg, vga_color_t bg);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_println(const char* str);
#endif
