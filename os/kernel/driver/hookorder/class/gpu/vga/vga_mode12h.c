#include "vga.h"

/*

	VGA mode 13h detector

*/
int vga_detect_mode_12h(void) {
	/*
		CRTC identify
	*/
    outb(VGA_CRTC_INDEX, 0x00);
    uint8_t horizontal_total = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_INDEX, 0x01);
    uint8_t horizontal_display_end = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_INDEX, 0x12);
    uint8_t vertical_display_end = inb(VGA_CRTC_DATA);
	/*
		Dump some tasty values
	*/
    #ifdef DEBUG
    printf("VGA_DETECT: Mode 12h check - HT=0x%x HDE=0x%x VDE=0x%x\r\n", horizontal_total, horizontal_display_end, vertical_display_end);
    #endif

    if (horizontal_total == 0x5F && horizontal_display_end == 0x4F && vertical_display_end == 0x8F) {
        return 1; /*Yes VGA*/
    }
    return 0; /*No VGA*/
}