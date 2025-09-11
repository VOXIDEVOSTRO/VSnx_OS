/*
	HEADER files
*/
#include "vga_text.h"
/*
	VGA DETECT TEXT MODE:
	as this couldnt be avaviable in some hardware	
*/
int vga_dec_text(void) {
    /*
		Check if the common addr is writabe and can be seen?
	*/
    volatile uint16_t* vga_mem = (volatile uint16_t*)VGA_TEXT_MEMORY;
    uint16_t original = vga_mem[0];
    vga_mem[0] = 0x4141; // 'A' with white on red
    /*
		Verify the write
	*/
    if (vga_mem[0] != 0x4141) {
        serial_print(COM1_PORT, "VGA_TEXT: Memory test failed\r\n");
        return 0; // ERROR
    }
    /*
		RESTORE
	*/
    vga_mem[0] = original;
    /*
		CHECK the CRTC registers
	*/
    outb(VGA_CRTC_INDEX, 0x0F); // Cursor location low
    uint8_t cursor_low = inb(VGA_CRTC_DATA);
	/*
		Test the write
	*/
    outb(VGA_CRTC_DATA, 0x55);
    if (inb(VGA_CRTC_DATA) != 0x55) {
        serial_print(COM1_PORT, "VGA_TEXT: CRTC test failed\r\n");
        return 0; // failure
    }
    /*
		restore cursor
	*/
    outb(VGA_CRTC_DATA, cursor_low);
    /*
		Check the status registers
	*/
    uint8_t status = inb(VGA_STATUS_REG);
    if (status == 0xFF) {
        serial_print(COM1_PORT, "VGA_TEXT: Status register invalid\r\n");
        return 0;
    }
    /*
		WE HAVE VGA TEXT MODE
	*/
    serial_print(COM1_PORT, "VGA_TEXT: Detection successful\r\n");
    return 1; // VGA text mode available
}