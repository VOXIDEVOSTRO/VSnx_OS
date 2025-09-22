#ifndef VGA_H
#define VGA_H
#include "../../../hookorder.h"
#include <stdint.h>
#include "../../../../../hal/io/io_ports.h"
/*
	VGA registers
*/
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
/*
	12h
*/
int vga_detect_mode_12h(void);
/*
	13h
*/
int vga_detect_mode_13h(void);
/*
	Order for VGA
*/
int vga_spawn_driver(void);
#endif