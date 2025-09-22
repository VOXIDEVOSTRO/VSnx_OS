/*
	HEADER FOLS
*/
#include "../../os/kernel/driver/hookcall/hookreg.h"
#include "../../os/kernel/driver/hook.h"
#include "../../os/kernel/syscalls/func.h" // for some user calls
#include <stdint.h>
/*
	UNCOMMENT for extra ouput!
*/


//#define DEBUG		/*UNCOMMENT!*/


/*
	prototypes
*/
int vga12h_driver_init(void);
void vga12h_driver_cleanup(void);
void vga12h_draw_pixel(int x, int y, uint8_t color);
void vga12h_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga12h_clear_screen(uint8_t color);
int api_vga12h_draw_pixel(int x, int y, uint8_t color);
int api_vga12h_fill_rect(int x, int y, int w, int h, uint8_t color);
int api_vga12h_set_mode(void);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
/*
	planer constants for 12h
*/
#define VGA_MODE12H_WIDTH 640
#define VGA_MODE12H_HEIGHT 480
#define VGA_MODE12H_BPP 4
#define VGA_MODE12H_PLANES 4
/*
	VGA registers
*/
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_IS1_RC 0x3DA
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
/*
	Memory
*/
#define VGA_MEMORY_BASE 0xA0000 /*simple Base for 12h*/
#define VGA_MEMORY_SIZE (64 * 1024/*64 kb*/)
/*
	12h state
*/
static struct {
    uint8_t* vga_memory;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint8_t current_plane;
} vga12h_state = {0};
/*
	register value array
*/
static const uint8_t vga_mode12h_seq[5] = {
    0x03/*reset*/,  
    0x01/*clockM*/,  
    0x0F/*map mask*/,
    0x00/*CMS*/,
    0x06/*mem mode*/ 
};

static const uint8_t vga_mode12h_crt[25] = {
    0x5F/*HT*/, 
    0x4F/*HDE*/,
    0x50/*HBS*/,
    0x82/*HBE*/,
    0x54/*HRS*/,
    0x80/*HRE*/,
    0x0D/*VT*/, 
    0x1E/*Overflow*/,
    0x00/*PRS*/,
    0x00/*MSL*/,
    0x00/* cursor >*/,
    0x00/* cursor <*/,
    0x00/*SAH*/,
    0x00/*SAL*/,
    0x00/*CLH*/,
    0x00/*CLL*/,
    0xEA/*VRS*/,
    0xEC/*VRE*/,
    0xDF/*VDE*/,
    0x28/*offset for scan line*/,
    0x00/*ULL*/,
    0xE0/*VBS*/,
    0x0D/*VBE*/,
    0xE3/*CRTC MC*/,
    0xFF/*Line cmp*/
};

// Graphics controller registers (controls graphics mode)
static const uint8_t vga_mode12h_gc[9] = {
    0x00,
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x00, 
    0x05/*misc*/,
    0x0F/*no color set*/,
    0xFF/*bit mask*/
};
static const uint8_t vga_mode12h_ac[21] = {
	/*
		Simple attr values
	*/
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
    0x01, 0x00, 0x0F, 0x00, 0x00
};
/*
	Simple inbuilt I/O ports
	No problem hence we gonna run in ring 0
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

	VGA register functions

*/
static void vga_write_seq(uint8_t index, uint8_t value) {
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
    inb(VGA_IS1_RC);  // Reset flip-flop
    outb(VGA_AC_INDEX, index);
    outb(VGA_AC_WRITE, value);
}
/*
	SET dac for the 12h too
*/
static void vga_set_dac_palette_12h(void) {
    #ifdef DEBUG
    print("[VGA12H] Setting DAC palette for mode 12h...");
    #endif
    outb(0x3C8, 0/*index*/);

    const uint8_t palette[16][3] = {
		/*
			So we have the beautiful 16
			palette here.
		*/
        {0, 0, 0},       // Black
        {0, 0, 42},      // Blue
        {0, 42, 0},      // Green
        {0, 42, 42},     // Cyan
        {42, 0, 0},      // Red
        {42, 0, 42},     // Magenta
        {42, 21, 0},     // Brown
        {42, 42, 42},    // Light Gray
        {21, 21, 21},    // Dark Gray
        {21, 21, 63},    // Light Blue
        {21, 63, 21},    // Light Green
        {21, 63, 63},    // Light Cyan
        {63, 21, 21},    // Light Red
        {63, 21, 63},    // Light Magenta
        {63, 63, 21},    // Yellow
        {63, 63, 63}     // White
    };

    for (int i = 0; i < 16; i++) {
        outb(0x3C9, palette[i][0]);
        outb(0x3C9, palette[i][1]);
        outb(0x3C9, palette[i][2]);
		/*RGB?*/
    }
    #ifdef DEBUG
    print("[VGA12H] DAC palette set");
    #endif
}
/*
	Set the plane duh...
*/
static void vga_select_plane(uint8_t plane) {
	/*
		Set the plane
	*/
    vga12h_state.current_plane = plane;
	/*
		Write acsesses on the plane
	*/
    vga_write_seq(0x02, 1 << plane)/*mappy mask*/;
    vga_write_gc(0x04, plane)/*RMS*/;
    vga_write_gc(0x01, 1 << plane)/*set/rset*/;    
}
/*
	THE heart of the init
*/
static void vga_set_mode_12h(void) {
    #ifdef DEBUG
    print("[VGA12H] Setting mode 12h");
    #endif
	/*
		Disable INTERRUPTS or ELSE....
	*/
    __asm__ volatile("cli");
	/*
		Hold up... wait a minute
	*/
    #ifdef DEBUG
    print("[VGA12H] Waiting for vertical retrace...");
    #endif
    while ((inb(VGA_IS1_RC) & 0x08));
    while (!(inb(VGA_IS1_RC) & 0x08));
	/*
		Unlock the CRTC. Normal VGA candy
	*/
    #ifdef DEBUG
    print("[VGA12H] Unlocking CRTC registers...");
    #endif
    outb(VGA_CRTC_INDEX, 0x11);
    uint8_t crtc11 = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_DATA, crtc11 & 0x7F);
	/*
		Also for misc
	*/
    #ifdef DEBUG
    print("[VGA12H] Setting misc register...");
    #endif
    outb(VGA_MISC_WRITE, 0xE3);
	/*
		da sequencer
	*/
    #ifdef DEBUG
    print("[VGA12H] Setting sequencer registers...");
    #endif
    for (int i = 0; i < 5; i++) {
        vga_write_seq(i, vga_mode12h_seq[i]);
    }
	/*
		Set CRTC
	*/
    #ifdef DEBUG
    print("[VGA12H] Setting CRTC registers...");
    #endif
    for (int i = 0; i < 25; i++) {
        vga_write_crt(i, vga_mode12h_crt[i]);
    }
	/*
		And the main thing. the graphic controller registers
	*/
    #ifdef DEBUG
    print("[VGA12H] Setting graphics controller registers...");
    #endif
    for (int i = 0; i < 9; i++) {
        vga_write_gc(i, vga_mode12h_gc[i]);
    }
	/*
		Set the attr registers too
		Man thats lots of register stuff
	*/
    #ifdef DEBUG
    print("[VGA12H] Setting attribute controller registers...");
    #endif
    for (int i = 0; i < 21; i++){
        vga_write_ac(i, vga_mode12h_ac[i]);
    }
	/*
		FINALLY enable Video. because we need to see something on screen
	*/
    #ifdef DEBUG
    print("[VGA12H] Enabling video output...because bro how the heck would you see graphics at all");
    #endif
    inb(VGA_IS1_RC);
    outb(VGA_AC_INDEX, 0x20);
	/*
		DAC up!
	*/
    vga_set_dac_palette_12h();
	/*
		Finally no more color overwrites and bleeds. a simple delay works
		it was pure luck
	*/
    for (volatile int i = 0; i < 100000; i++) {
        __asm__ volatile("nop");
    }
	/*
		Set the planes to 0
		or just clear the screen
	*/
    #ifdef DEBUG
    print("[VGA12H] Clearing screen...");
    #endif
    for (int plane = 0; plane < VGA_MODE12H_PLANES; plane++) {
        vga_select_plane(plane);
        for (int i = 0; i < VGA_MEMORY_SIZE; i++) {
            vga12h_state.vga_memory[i] = 0;
        }
    }
	/*
		Re enable
	*/
    __asm__ volatile("sti");
	/*
		UPDOOTE DA DRIIVEER
	*/
    vga12h_state.width = VGA_MODE12H_WIDTH;
    vga12h_state.height = VGA_MODE12H_HEIGHT;
    vga12h_state.bpp = VGA_MODE12H_BPP;

    #ifdef DEBUG
    print("[VGA12H] Mode 12h set and ready to do some shi");
    #endif
}
/*

	I guess too much complicated drawing...
	may try to scrap them later

*/
void vga12h_draw_pixel(int x, int y, uint8_t color) {
	/*
		"VALIDATE" hits HARD
	*/
    if (x < 0 || x >= vga12h_state.width || y < 0 || y >= vga12h_state.height) {
        return;
    }
	/*
		Set the bits and mask
	*/
    uint16_t offset = (y * 80) + (x / 8);
    uint8_t bit = 7 - (x % 8);
    uint8_t mask = (uint8_t)(1u << bit);
	/*
		Finally write the pixels
	*/
    for (int plane = 0; plane < VGA_MODE12H_PLANES; plane++) {
        /*
			Write to target plane.
			First read map select and write enable
			the plane.
		*/
        vga_write_gc(0x04, (uint8_t)plane);
        vga_write_seq(0x02, (uint8_t)(1u << plane));
		/*
			Da VRAM and thats misconception. its a frambuffer... right?
		*/
        volatile uint8_t* vram = (volatile uint8_t*)(0xA0000 + offset);
		/*
			LATCH read
		*/
        volatile uint8_t latched = *vram;
		/*
			Perform a RMW.
		*/
        uint8_t new_byte = (uint8_t)latched;
        if (color & (1u << plane)) {
            new_byte |= mask;
        } else {
            new_byte &= (uint8_t)~mask;
        }
		/*
			Write back
		*/
        *vram = new_byte;
    }
}
/*
	Simple rectangle
*/
void vga12h_draw_rect(int x, int y, int w, int h, uint8_t color) {
	/*
		Make sure of the X
	*/
    if (x < 0) { w += x; x = 0; }
	/*
		Then the Y
	*/
    if (y < 0) { h += y; y = 0; }
	/*
		As well as the bounds
	*/
    if (x + w > vga12h_state.width) w = vga12h_state.width - x;
    if (y + h > vga12h_state.height) h = vga12h_state.height - y;
	/*
		And NULL check the Width and high
	*/
    if (w <= 0 || h <= 0) return;
	/*
		Finally draw
	*/
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vga12h_draw_pixel(j, i, color);
        }
    }
}
/*
	screen clear
*/
void vga12h_clear_screen(uint8_t color) {
	/*
		Just Fill that plane
	*/
    for (int plane = 0; plane < VGA_MODE12H_PLANES; plane++) {
        vga_select_plane(plane);
        uint8_t fill_value = (color & (1 << plane)) ? 0xFF : 0x00;
        for (int i = 0; i < VGA_MEMORY_SIZE; i++) {
            vga12h_state.vga_memory[i] = fill_value;
        }
    }
}
/*
	THE CAPTAIN HOOK functions. for API
*/
void vga12h_draw_pixel_hook(void *data) {
	/*
		UNPACK
	*/
    struct hook_data *d = (struct hook_data*)data;
    int x = (int)d->args[0];
    int y = (int)d->args[1];
    uint8_t color = (uint8_t)d->args[2];
	/*
		Call
	*/
    vga12h_draw_pixel(x, y, color);
}
void vga12h_fill_rect_hook(void *data) {
	/*
		UNPACK
	*/
    struct hook_data *d = (struct hook_data*)data;
    int x = (int)d->args[0];
    int y = (int)d->args[1];
    int w = (int)d->args[2];
    int h = (int)d->args[3];
    uint8_t color = (uint8_t)d->args[4];
	/*
		CALL
	*/
    vga12h_draw_rect(x, y, w, h, color);
}
/*
	MAIN driver init
*/
int vga12h_driver_init(void) {
    /*
		MAP VGA
	*/
    vga12h_state.vga_memory = (uint8_t*)0xA0000/*Standard base*/;
	/*
		Set and init the mode
	*/
    vga_set_mode_12h();
	/*
		REG THE HOOKS!
	*/
    reg_hook("gfx_draw_pixel", vga12h_draw_pixel_hook);
    reg_hook("gfx_fill_rect", vga12h_fill_rect_hook);
    return 0;
}
void vga12h_driver_cleanup(void) {
	/*
		Cleanup.
	*/
    vga_write_seq(0x00, 0x03);
    vga_write_crt(0x17, 0x80);
}
/*

	ENTRY point of the driver

*/
void _start(void) {
    #ifdef DEBUG
    print("[VGA12H] Starting VGA Mode 12h driver...");
    #endif
    if (vga12h_driver_init()/*MAin init is here*/ == 0) {
        #ifdef DEBUG
        print("[VGA12H] Driver initialized successfully");
        #endif
		/*
			Clean up artifacts
		*/
        vga12h_clear_screen(0);
        #ifdef DEBUG
        print("[VGA12H] VGA Mode 12h driver is done");
        #endif
		t_block();
    } else {
		/*
			Failure
		*/
        #ifdef DEBUG
        print("[VGA12H] Driver initialization failed BECAUSE YOU FAILURE");
        #endif
		t_block();
    }
}
