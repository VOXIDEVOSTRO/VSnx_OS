#ifndef VGA_GFX_H
#define VGA_GFX_H
/*
	Headerz
*/
#include "../../../elf/elf.h"
#include "../../../elf/process.h"
#include "../../../syscalls/syscall.h"
#include "../../../threading/thread.h"
#include "../../../driver/hook.h"
#include <stdint.h>
#include "../../io/io_ports.h"
/*
	VGA registers
*/
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
/*
	prototypes
*/
int vga_detect_mode_12h(void);
int vga_detect_mode_13h(void);
void vga_load_graphics_driver(void);
#endif
