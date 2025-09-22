#include "vga.h"
/*

	Detect 13h

*/
int vga_detect_mode_13h(void) {
	/*
		Simple CRTC identification
	*/
    outb(VGA_CRTC_INDEX, 0x00);
    uint8_t horizontal_total = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_INDEX, 0x01);
    uint8_t horizontal_display_end = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_INDEX, 0x12);
    uint8_t vertical_display_end = inb(VGA_CRTC_DATA);
	/*
		Some dumplings
	*/
    #ifdef DEBUG
    printf("VGA_DETECT: Mode 13h check - HT=0x%x HDE=0x%x VDE=0x%x\r\n", horizontal_total, horizontal_display_end, vertical_display_end);
    #endif

    if (horizontal_total == 0x27 && horizontal_display_end == 0x1F && vertical_display_end == 0x8F) {
        return 1; /*is prsent*/
    }
    return 0; /*no VGA*/
}
