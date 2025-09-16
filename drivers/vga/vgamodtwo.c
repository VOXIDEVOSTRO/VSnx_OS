/*

	idk but why this much big space.
	so this is a lightweight VGA
	Graphics Driver, For mode 13h,
	Extremely small i know but i
	don't think the most realiable.
	but yet i think its extremely
	useful. to demostrate our hook
	system. well still very early 
	stage, and may add double buffering
	or somthing for better performance.
	this driver is more for compatiablity
	for older ancient stuff such as VGA

*/
/*
	Header files
*/
/*
	This is for the static inlines for the reg_hook
*/
#include "../../os/kernel/driver/hookcall/hookreg.h"
/*
	Some defines here like the pointer and packing of args
*/
#include "../../os/kernel/driver/hook.h"
/*
	This is typically for userspace but our syscall allows it somehow
	useful for allocing memory
*/
#include "../../os/kernel/syscalls/func.h"
/*
	Some standard libs
*/
#include <stdint.h>
/*
	PROTOTYPES (to avoid compiler)
*/
int vga13h_driver_init(void);
void vga13h_driver_cleanup(void);
void vga13h_draw_pixel(int x, int y, uint8_t color);
void vga13h_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga13h_clear_screen(uint8_t color);
int api_vga13h_draw_pixel(int x, int y, uint8_t color);
int api_vga13h_fill_rect(int x, int y, int w, int h, uint8_t color);
int api_vga13h_set_mode(void);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
/*

	VGA macros or constants

*/
#define VGA_MODE13H_WIDTH   320
#define VGA_MODE13H_HEIGHT  200
#define VGA_MODE13H_BPP     8
#define VGA_MODE13H_SIZE    (VGA_MODE13H_WIDTH * VGA_MODE13H_HEIGHT) /*64,000 bytes*/
/*
	Some registers
*/
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA  0x3C5
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_GC_INDEX  0x3CE
#define VGA_GC_DATA   0x3CF
#define VGA_AC_INDEX  0x3C0
#define VGA_AC_WRITE  0x3C0
#define VGA_AC_READ   0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_IS1_RC    0x3DA
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9
#define VGA_MEMORY_BASE 0xA0000
/*
	The 13h STATE structure
*/
static struct {
    volatile uint8_t* vga_memory;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} vga13h_state = {0};
/*
	So 13h register value array:
	So if i am right the resoultion is 320x200
	and abouse 256 colors.
	and is a linear

	0x3 = reset, 0x01 clock mode, 0x0F = map mask for planes,
	0x00 = the char map, 0x0E can be mem mode.

*/
static const uint8_t vga_mode13h_seq[5] = {
    0x03/*reset*/,0x01/*clock*/,0x0F/*map mask*/,0x00/*char map*/,0x0E/*mem mode*/
};

static const uint8_t vga_mode13h_crt[25] = {
    0x5F/*HT*/,
    0x4F/*HDE*/,
    0x50/*HBS*/,
    0x82/*HBE*/,
    0x54/*HRS*/,
    0x80/*HRE*/,
    0xBF/*VT*/, 
    0x1F/*Overflow*/,
    0x00/*PRS*/,
    0x41/*MSL*/,
    0x00/*Cursor >*/,
    0x00/*Cursor <*/,
    0x00/*SAH*/, 
    0x00/*SAL*/,
    0x00/*CLH*/,  
    0x00/*CLL*/,  
    0x9C/*VRS*/,
    0x0E/*VRE*/,  
    0x8F/*VDE*/,  
    0x28/*offset for scanline*/,  
    0x40/*ULL*/,
    0x96/*VBS*/,
    0xB9/*VBLNKE*/,
    0xA3/*CRTC MC*/,
    0xFF/*Line cmp*/
};

static const uint8_t vga_mode13h_gc[9] = {
    0x00/*rset/set*/,
    0x00/*enable the above*/,
    0x00/*color cmp*/,
    0x00/*no rotate*/, 
    0x00/*RMS*/,
    0x40/*GFX mode*/,
    0x05/*Misc*/,
    0x0F/*NULL USE*/, 
    0xFF/*Bit mask*/   
};

static const uint8_t vga_mode13h_ac[21] = {
	/*
		pallete registers
	*/
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 
    0x00
};
/*
	some inbuilt I/O functions
*/
void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}
uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outl(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}
/*

	VGA registers

*/
static/*keep em static*/ void vga_write_seq(uint8_t index, uint8_t value) {
    outb(VGA_SEQ_INDEX, index);
    outb(VGA_SEQ_DATA, value);
}
static void vga_write_crt(uint8_t index, uint8_t value) {
    outb(VGA_CRTC_INDEX, index);
    outb(VGA_CRTC_DATA, value);
}

static void vga_write_gc(uint8_t index, uint8_t value) {
    outb(VGA_GC_INDEX, index);
    outb(VGA_GC_DATA, value);
}

static void vga_write_ac(uint8_t index, uint8_t value) {
    inb(VGA_IS1_RC);/*flippy floppy*/
    outb(VGA_AC_INDEX, index);
    outb(VGA_AC_WRITE, value);
}

static void vga13h_set_dac_default(void) {
    print("[VGA13H] Setting DAC palette for 256-color mode (grayscale base)...\n"/*using grey scale for now here*/);
	/*
		Classy VGA palette
	*/
    const uint8_t base16[16][3] = {
        {0, 0, 0}, {0, 0, 42}, {0, 42, 0}, {0, 42, 42},
        {42, 0, 0}, {42, 0, 42}, {42, 21, 0}, {42, 42, 42},
        {21, 21, 21}, {21, 21, 63}, {21, 63, 21}, {21, 63, 63},
        {63, 21, 21}, {63, 21, 63}, {63, 63, 21}, {63, 63, 63}
    };
    outb(VGA_DAC_WRITE_INDEX, 0);
    for (int i = 0; i < 16; i++) {
        outb(VGA_DAC_DATA, base16[i][0]);
        outb(VGA_DAC_DATA, base16[i][1]);
        outb(VGA_DAC_DATA, base16[i][2]);
    }
	/*
		Fill up the rest
	*/
    for (int i = 16; i < 256; i++) {
        uint8_t v = (uint8_t)((i * 63) / 255);
        outb(VGA_DAC_DATA, v);
        outb(VGA_DAC_DATA, v);
        outb(VGA_DAC_DATA, v);
    }
    print("[VGA13H] DAC palette set\n");
}
/*

	THE MAIn init process

*/
static void vga_set_mode_13h(void) {
    print("[VGA13H] Setting mode 13h (320x200x256 packed)...\n");
	/*
		bye bye interrupts
	*/
    __asm__ volatile("cli");
	/*
		Use vertial retrace
	*/
    print("[VGA13H] Waiting for vertical retrace...\n");
    while ((inb(VGA_IS1_RC) & 0x08));
    while (!(inb(VGA_IS1_RC) & 0x08));
	/*
		Setup misc
	*/
    outb(VGA_MISC_WRITE, 0x63);
	/*
		Seq or sequencer
	*/
    for (int i = 0; i < 5; i++) {
        vga_write_seq(i, vga_mode13h_seq[i]);
    }
	/*
		Most important, UNLOCK CRTC
	*/
    outb(VGA_CRTC_INDEX, 0x11);
    uint8_t crtc11 = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_DATA, (uint8_t)(crtc11 & 0x7F));
	/*
		CRTC
	*/
    for (int i = 0; i < 25; i++) {
        vga_write_crt(i, vga_mode13h_crt[i]);
    }
	/*
		GFX controller
	*/
    for (int i = 0; i < 9; i++) {
        vga_write_gc(i, vga_mode13h_gc[i]);
    }
	/*
		attr controller
	*/
    for (int i = 0; i < 21; i++) {
        vga_write_ac(i, vga_mode13h_ac[i]); /*Use the above tables*/
    }
	/*
		Enable video for some output
	*/
    inb(VGA_IS1_RC);
    outb(VGA_AC_INDEX, 0x20);
	/*
		Set the DAC
	*/
    vga13h_set_dac_default();
	/*
		Welcome back
	*/
    __asm__ volatile("sti");
	/*
		Update teh driver
	*/
    vga13h_state.width = VGA_MODE13H_WIDTH;
    vga13h_state.height = VGA_MODE13H_HEIGHT;
    vga13h_state.bpp = VGA_MODE13H_BPP;
    vga13h_state.vga_memory = (volatile uint8_t*)VGA_MEMORY_BASE;
    print("[VGA13H] Mode 13h set done\n");
}
/*
	BASIC drawing
*/
void vga13h_draw_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= vga13h_state.width || y < 0 || y >= vga13h_state.height) {
        return;
    }
    vga13h_state.vga_memory[y * vga13h_state.width + x] = color;
}
void vga13h_draw_rect(int x, int y, int w, int h, uint8_t color) {
	/*
		Avoid overflow
	*/
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > vga13h_state.width) w = vga13h_state.width - x;
    if (y + h > vga13h_state.height) h = vga13h_state.height - y;
    if (w <= 0 || h <= 0) return;

    for (int row = 0; row < h; row++) {
        int yy = y + row;
        int base = yy * vga13h_state.width + x;
        for (int col = 0; col < w; col++) {
            vga13h_state.vga_memory[base + col] = color;
        }
    }
}
void vga13h_clear_screen(uint8_t color) {
    for (int i = 0; i < VGA_MODE13H_SIZE/*64K bytes*/; i++) {
        vga13h_state.vga_memory[i] = color;
    }
}
/*
	CAPTAIN HOOK!
*/
void vga13h_draw_pixel_hook(void *data) {
	/*
		Unpack
	*/
    struct hook_data *d = (struct hook_data*)data;
    int x = (int)d->args[0];
    int y = (int)d->args[1];
    uint8_t color = (uint8_t)d->args[2];
	/*
		Draw
	*/
    vga13h_draw_pixel(x, y, color);
}
void vga13h_fill_rect_hook(void *data) {
	/*
		Unpack
	*/
    struct hook_data *d = (struct hook_data*)data;
    int x = (int)d->args[0];
    int y = (int)d->args[1];
    int w = (int)d->args[2];
    int h = (int)d->args[3];
    uint8_t color = (uint8_t)d->args[4];
	/*
		Draw
	*/
    vga13h_draw_rect(x, y, w, h, color);
}
/*
	MAIN driver init
*/
int vga13h_driver_init(void) {
	/*
		Mapp up
	*/
    vga13h_state.vga_memory = (volatile uint8_t*)VGA_MEMORY_BASE/*0xA0000 if i am right*/;
	/*
		Set
	*/
    vga_set_mode_13h();
	/*
		Register hooks
	*/
    reg_hook("gfx_draw_pixel", vga13h_draw_pixel_hook);
    reg_hook("gfx_fill_rect", vga13h_fill_rect_hook);
	/* We dont need this
    reg_hook("gfx_set_mode", vga13h_set_mode_hook);
	*/
    return 0;
}
/*
	Incase of failure or driver not needed
*/
void vga13h_driver_cleanup(void) {
	/*
		Fall back
	*/
    vga_write_seq(0x00, 0x03);
    vga_write_crt(0x17, 0x80);
}
/*

	THE driver entry point
	mostly contains test.

*/
void _start(void) {
    print("[VGA13H] Starting VGA Mode 13h driver...\n");
	/*
		Main driver init
	*/
    if (vga13h_driver_init() == 0) {
        print("[VGA13H] Driver initialized successfully\n");
		/*
			Clear up
		*/
		vga13h_clear_screen(0); 
		/*
			ALIVE
		*/
        while (1) {
            __asm__ volatile("nop"/*Noperation*/);
        }
    } else {
        print("[VGA13H] Driver initialization failed!\n");
    }
	/*
		If driver failed to init
	*/
    while (1) {
        __asm__ volatile("nop"/*Nooperation*/);
    }
}