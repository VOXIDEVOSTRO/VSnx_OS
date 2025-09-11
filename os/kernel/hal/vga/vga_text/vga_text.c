/*
    HEADER files
*/
#include "vga_text.h"
#include "../../serial_com/serial.h"
#include "../../io/io_ports.h"
/*
	Vga defines: declaring the vars
*/
volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_TEXT_MEMORY; /* 0xB8000 */
int cursor_x = 0; // Top left corner of the screen
int cursor_y = 0;
uint8_t current_color = 0x0F; // White on black
/*
	Make Color
*/
uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
	// fg = foreground, bg = background
    return fg | bg << 4;
}
/*
    MAKE VGA ENTRY (char + color)
*/
uint16_t vga_make_entry(char c, uint8_t color) {
	/*
		Color the char
	*/
    return (uint16_t)c | (uint16_t)color << 8;
}
/*
    UPDATE HARDWARE CURSOR
*/
void vga_update_cursor(void) {
	/*
		Update pos
	*/
    uint16_t pos = cursor_y * VGA_TEXT_WIDTH + cursor_x;
	/*
		Move the cursor
	*/
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
/*
    SCROLL SCREEN UP
*/
void vga_scroll(void) {
    for (int y = 0; y < VGA_TEXT_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_TEXT_WIDTH; x++) {
            vga_buffer[y * VGA_TEXT_WIDTH + x] = vga_buffer[(y + 1) * VGA_TEXT_WIDTH + x];
        }
    }
    /*
		CLEAR LAST LINE
	*/
    for (int x = 0; x < VGA_TEXT_WIDTH; x++) {
        vga_buffer[(VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH + x] = vga_make_entry(' ', current_color);
    }
    cursor_y = VGA_TEXT_HEIGHT - 1; /*bring back to the bound of screen*/
}
/*
    INIT VGA TEXT MODE
*/
int vga_text_init(void) {
	/*
		Check first do we have VGA text at all?
	*/
	if(vga_dec_text() == 1) {
		/*
			initialize the default color
		*/
    	current_color = vga_make_color(VGA_WHITE, VGA_BLACK);
		/*
			initialize the cursor
		*/
    	cursor_x = 0;
    	cursor_y = 0;
		/*
			Clear the artifacts
		*/
    	vga_text_clear();
    	serial_print(COM1_PORT, "VGA_TEXT: Initialized\r\n"); // Done
		return 0;
	} else {
		/*
			If non - continue
		*/
		serial_print(COM1_PORT, "VGA_TEXT: Failed to initialize, No VGA text available.\r\n");
		return -1;
	}
}
/*
	Clear the screen
*/
void vga_text_clear(void) {
	/*
		Fill the buffer completely with empty spaces
	*/
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++) {
        vga_buffer[i] = vga_make_entry(' ', current_color);
    }
	/*
		Reset the cursor
	*/
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor();
}
/*
    SET COLOR
*/
void vga_text_set_color(vga_color_t fg, vga_color_t bg) {
	/*
		To change default color
	*/
    current_color = vga_make_color(fg, bg);
}
/*
    PUT CHARACTER (PUTS)
*/
void vga_putchar(char c) {
	/*
		Handle \n / Newline characters
	*/
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
	/*
		handle the \r / reset character
		brings the cursor back to the first col'
	*/
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
	/*
		Handle normal chars like ASCII ones
	*/
        vga_buffer[cursor_y * VGA_TEXT_WIDTH + cursor_x] = vga_make_entry(c, current_color);
        cursor_x++;
    }
    /*
		Handle both direction scrolls
	*/
    if (cursor_x >= VGA_TEXT_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_TEXT_HEIGHT) {
        vga_scroll();
    }
    /*
		Update
	*/
    vga_update_cursor();
	return 0;
}
/*
    PRINT STRING
*/
void vga_print(const char* str) {
	/*
		Form the string
	*/
    while (*str) {
        vga_putchar(*str++);
    }
}
/*
    PRINT LINE
*/
void vga_println(const char* str) {
    vga_print(str);
    vga_putchar('\n');
}
