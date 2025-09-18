/*
	Header files
*/
#include "vga_gfx.h"
#include "../../../utilities/utility.h"/*for printf*/
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

void vga_load_graphics_driver(void) {
    #ifdef DEBUG
    printf("VGA_GFX: Starting graphics driver detection...\r\n");
    #endif
	/*
		check for both modes
	*/
    int mode13 = vga_detect_mode_13h();
    #ifdef DEBUG
    printf("VGA_DETECT: Mode 13h result = %d\r\n", mode13);
    #endif

    int mode12 = vga_detect_mode_12h();
    #ifdef DEBUG
    printf("VGA_DETECT: Mode 12h result = %d\r\n", mode12);
    #endif
	/*
		Load the respective driver (Respect +1)
	*/
    if (mode13) {
        #ifdef DEBUG
        serial_print(COM1_PORT, "VGA_GFX: Mode 13h detected, loading driver...\r\n");
        #endif
        process_t* mod13 = spawn_process("/MODULES/SYS/DRIVERS/VGA/VGAMODTWO.ELF", THREAD_RING0/*This is 13h*/, THREAD_PRIORITY_LOW);
		execute_process(mod13); // execute DUH...
    } if (mode12) {
        #ifdef DEBUG
        serial_print(COM1_PORT, "VGA_GFX: Mode 12h detected, loading driver...\r\n");
        #endif
        process_t* mod12 = spawn_process("/MODULES/SYS/DRIVERS/VGA/VGAMODONE.ELF", THREAD_RING0/*This is prolly 12h*/, THREAD_PRIORITY_LOW);
		execute_process(mod12);
    } else {
        #ifdef DEBUG
        serial_print(COM1_PORT, "VGA_GFX: No VGA graphics mode detected.\r\n");
        #endif
    }
}