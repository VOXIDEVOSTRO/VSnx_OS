/*
	Heads files
	Wait its jsut exclusion for prototypes...
*/
#define INUSE
/*
	here is the one
*/
#include "vboxsvga.h"
/*
	UNCOMMENT for debug output...
*/

//#define DEBUG /*UNCOMMENT IF EXTRA!*/

/*
	Some debug essentials...
*/
#ifdef DEBUG
void print_str(const char *s) {
    #ifdef DEBUG
    print(s);
    #endif
}

// Print a 32-bit hex value
void print_hex32(uint32_t val) {
    static const char hex[] = "0123456789ABCDEF";
    char buf[11]; // "0x" + 8 digits + '\0'
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        buf[2 + i] = hex[(val >> (28 - 4*i)) & 0xF];
    }
    buf[10] = '\0';
    #ifdef DEBUG
    print(buf);
    #endif
}

// Print a 16-bit hex value
void print_hex16(uint16_t val) {
    static const char hex[] = "0123456789ABCDEF";
    char buf[7]; // "0x" + 4 digits + '\0'
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 4; i++) {
        buf[2 + i] = hex[(val >> (12 - 4*i)) & 0xF];
    }
    buf[6] = '\0';
    #ifdef DEBUG
    print(buf);
    #endif
}

/* Helper functions for printing numbers */
void print_num(uint32_t num) {
    if (num == 0) {
        #ifdef DEBUG
        print("0");
        #endif
        return;
    }
    char buf[16];
    int i = 0;
    uint32_t temp = num;
    while (temp > 0) {
        buf[i++] = '0' + (temp % 10);
        temp /= 10;
    }
    while (i > 0) {
        char c[2] = {buf[--i], '\0'};
        #ifdef DEBUG
        print(c);
        #endif
    }
}

void print_hex_num(uint32_t num, int digits) {
    char buf[16];
    int i = 0;
    uint32_t temp = num;
    if (temp == 0 && digits > 0) {
        for (int j = 0; j < digits; j++) {
            buf[i++] = '0';
        }
    } else {
        while (temp > 0 || i < digits) {
            if (i >= digits && temp == 0) break;
            int digit = temp % 16;
            buf[i++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
            temp /= 16;
        }
    }
    while (i > 0) {
        char c[2] = {buf[--i], '\0'};
        #ifdef DEBUG
        print(c);
        #endif
    }
}
#endif
/*
	I/O ports...
	copy pasted... from my kernel code... lol
*/
/*
	[B] IO_PORTS
*/
void outb(uint16_t port, uint8_t data) { // OUT
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
uint8_t inb(uint16_t port) { // IN
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
/*
	[W] PORTS
*/
uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}
/*
	[L] PORTS
*/
uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outl(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}
/*


	PCI hook wrappers for CONFIG and stuff 


*/
uint32_t pci_config_read_dword(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off;
    int64_t ret = hook_call_ret("pci_config_read_dword", &d);
    return (uint32_t)ret;
}
uint16_t pci_config_read_word(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off;
    int64_t ret = hook_call_ret("pci_config_read_word", &d);
    return (uint16_t)ret;
}
uint8_t pci_config_read_byte(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off;
    int64_t ret = hook_call_ret("pci_config_read_byte", &d);
    return (uint8_t)ret;
}

void pci_config_write_dword(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off, uint32_t val) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off; d.args[4]=val;
    (void)hook_call_ret("pci_config_write_dword", &d);
}
void pci_config_write_word(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off, uint16_t val) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off; d.args[4]=val;
    (void)hook_call_ret("pci_config_write_word", &d);
}
void pci_config_write_byte(uint8_t bus, uint8_t dev, uint8_t fn, uint8_t off, uint8_t val) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn; d.args[3]=off; d.args[4]=val;
    (void)hook_call_ret("pci_config_write_byte", &d);
}

/*


	Devices and Buses


*/

pci_device_t *pci_device_create(uint8_t bus, uint8_t dev, uint8_t fn) {
    struct hook_data d = {0};
    d.args[0]=bus; d.args[1]=dev; d.args[2]=fn;
    int64_t ret = hook_call_ret("pci_device_create", &d);
    return (pci_device_t*)RET_PTR(ret);
}
void pci_device_destroy(pci_device_t *device) {
    struct hook_data d = {0};
    d.args[0]=U64P(device);
    (void)hook_call_ret("pci_device_destroy", &d);
}
void pci_device_read_config(pci_device_t *device) {
    struct hook_data d = {0};
    d.args[0]=U64P(device);
    (void)hook_call_ret("pci_device_read_config", &d);
}

pci_bus_t *pci_bus_create(uint8_t bus_number) {
    struct hook_data d = {0};
    d.args[0]=bus_number;
    int64_t ret = hook_call_ret("pci_bus_create", &d);
    return (pci_bus_t*)RET_PTR(ret);
}
void pci_bus_destroy(pci_bus_t *bus) {
    struct hook_data d = {0};
    d.args[0]=U64P(bus);
    (void)hook_call_ret("pci_bus_destroy", &d);
}
void pci_bus_scan_devices(pci_bus_t *bus) {
    struct hook_data d = {0};
    d.args[0]=U64P(bus);
    (void)hook_call_ret("pci_bus_scan_devices", &d);
}

/*


	High controls


*/

void pci_enumerate(void) { struct hook_data d={0}; (void)hook_call_ret("pci_enumerate",&d); }
void pci_init(void)      { struct hook_data d={0}; (void)hook_call_ret("pci_init",&d); }
void pci_shutdown(void)  { struct hook_data d={0}; (void)hook_call_ret("pci_shutdown",&d); }

/*


	Find and qeury related stuff


*/

pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    struct hook_data d = {0};
    d.args[0]=vendor_id; d.args[1]=device_id;
    int64_t ret = hook_call_ret("pci_find_device", &d);
    return (pci_device_t*)RET_PTR(ret);
}
pci_device_t *pci_find_device_by_class(uint8_t class_code, uint8_t subclass) {
    struct hook_data d = {0};
    d.args[0]=class_code; d.args[1]=subclass;
    int64_t ret = hook_call_ret("pci_find_device_by_class", &d);
    return (pci_device_t*)RET_PTR(ret);
}
pci_device_t *pci_get_devices(void) {
    struct hook_data d = {0};
    int64_t ret = hook_call_ret("pci_get_devices", &d);
    return (pci_device_t*)RET_PTR(ret);
}

/*


	BARS. (64 bit ones?)


*/

uint64_t pci_get_bar_address(pci_device_t *device, int bar_index) {
    struct hook_data d = {0};
    d.args[0]=U64P(device); d.args[1]=(uint64_t)bar_index;
    int64_t ret = hook_call_ret("pci_get_bar_address", &d);
    return (uint64_t)ret;
}
uint64_t pci_get_bar_size(pci_device_t *device, int bar_index) {
    struct hook_data d = {0};
    d.args[0]=U64P(device); d.args[1]=(uint64_t)bar_index;
    int64_t ret = hook_call_ret("pci_get_bar_size", &d);
    return (uint64_t)ret;
}
_Bool pci_bar_is_io(pci_device_t *device, int bar_index) {
    struct hook_data d = {0};
    d.args[0]=U64P(device); d.args[1]=(uint64_t)bar_index;
    int64_t ret = hook_call_ret("pci_bar_is_io", &d);
    return (_Bool)(ret != 0);
}
_Bool pci_bar_is_64bit(pci_device_t *device, int bar_index) {
    struct hook_data d = {0};
    d.args[0]=U64P(device); d.args[1]=(uint64_t)bar_index;
    int64_t ret = hook_call_ret("pci_bar_is_64bit", &d);
    return (_Bool)(ret != 0);
}

/*


	Massive code block of toggles


*/

/*
	Mastering
*/
void pci_enable_bus_mastering(pci_device_t *device)      { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_enable_bus_mastering",&d); }
void pci_disable_bus_mastering(pci_device_t *device)     { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_disable_bus_mastering",&d); }
/*
	I/O
*/
void pci_enable_io_space(pci_device_t *device)           { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_enable_io_space",&d); }
void pci_disable_io_space(pci_device_t *device)          { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_disable_io_space",&d); }
/*
	MEM
*/
void pci_enable_memory_space(pci_device_t *device)       { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_enable_memory_space",&d); }
void pci_disable_memory_space(pci_device_t *device)      { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_disable_memory_space",&d); }
/*
	Interrupts
*/
void pci_enable_interrupts(pci_device_t *device)         { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_enable_interrupts",&d); }
void pci_disable_interrupts(pci_device_t *device)        { struct hook_data d={0}; d.args[0]=U64P(device); (void)hook_call_ret("pci_disable_interrupts",&d); }

/*


	Globals for VBOXSVGA driver
	or frambuffer and mains and stuff


*/
volatile uint32_t *framebuffer;
uint32_t pitch;
static volatile uint32_t *g_fb = 0;
static int g_w = 0, g_h = 0, g_pitch = 0, g_bpp = 32;
static int g_fmt = 1/*8 RGB*/;

/*


	Kmodes table


*/
static const svga_mode_t k_modes[] = {
    {0, 640, 480, 32, 1},
    {1, 800, 600, 32, 1},
    {2, 1024, 768, 32, 1},
    {3, 1280, 720, 32, 1},
    {4, 1280, 800, 32, 1},
    {5, 1280, 1024, 32, 1},
    {6, 1366, 768, 32, 1},
    {7, 1440, 900, 32, 1},
    {8, 1600, 900, 32, 1},
    {9, 1680, 1050, 32, 1},
    {10, 1920, 1080, 32, 1},
};
/*
	Main count
*/
static const int k_modes_count = sizeof(k_modes)/sizeof(k_modes[0]);

/*


	MAIN init of the driver


*/
int vboxsvga_init(void) {
	/*
		Init the variables
	*/
    uint8_t found_bus=0, found_dev=0, found_fn=0;
    int found = 0;
	/*
		Find up the device
	*/
    for (uint8_t bus=0; bus<1; bus++) {
		/*
			We only check BUS 0,
			may add for the others later
		*/
        for (uint8_t dev=0; dev<32; dev++) {
			/*
				Read and find up!
			*/
            for (uint8_t fn=0; fn<8; fn++) {
                uint16_t vid = pci_config_read_word(bus, dev, fn, 0x00);
                uint16_t did = pci_config_read_word(bus, dev, fn, 0x02);
				/*
					Standard for VBOXSVGA
				*/
				#define VID 0x80EE
				#define DID 0xBEEF
				/*
					Match it up
				*/
                if (vid == VID && did == DID) {
					/*
						If non do NULL
					*/
                    found_bus=bus; found_dev=dev; found_fn=fn;
                    found=1; break;
                }
            }
            if (found) break;
        }
        if (found) break;
    }
	/*
		If not...
		Die
	*/
    if (!found) {
        #ifdef DEBUG
        print_str("[VBoxSVGA] Device not found\n");
        #endif
        return -1;
    }
	/*
		toggle one the I/O
		and memory for the frambuffer
	*/
    uint16_t cmd = pci_config_read_word(found_bus, found_dev, found_fn, 0x04);
    cmd |= 0x0003;
    pci_config_write_word(found_bus, found_dev, found_fn, 0x04, cmd);
	/*
		READ THE BARS
	*/
    uint32_t bar0 = pci_config_read_dword(found_bus, found_dev, found_fn, 0x10);
    uintptr_t fb_phys = (uintptr_t)(bar0 & 0xFFFFFFF0);
	/*
		Also debug up...
	*/
	#ifdef DEBUG
    print_str("[VBoxSVGA] BAR0="); print_hex32(bar0); print_str("\n");
	#endif
	/*
		SET the mode UP!
		Also i have to check the VBOX adapter source...
		https://www.virtualbox.org/browser/vbox/trunk/src/VBox/Devices/Graphics/DevVGA.cpp
	*/
    VBE_WRITE(VBE_DISPI_INDEX_ENABLE, 0);
    VBE_WRITE(VBE_DISPI_INDEX_XRES, 1024);
    VBE_WRITE(VBE_DISPI_INDEX_YRES, 768);
    VBE_WRITE(VBE_DISPI_INDEX_BPP,  32);
    VBE_WRITE(VBE_DISPI_INDEX_VIRT_WIDTH,  1024);
    VBE_WRITE(VBE_DISPI_INDEX_VIRT_HEIGHT, 768);
    VBE_WRITE(VBE_DISPI_INDEX_X_OFFSET, 0);
    VBE_WRITE(VBE_DISPI_INDEX_Y_OFFSET, 0);
    VBE_WRITE(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED | VBE_DISPI_NOCLEARMEM);
	/*
		read up all information
	*/
    uint16_t rd_w = VBE_READ(VBE_DISPI_INDEX_XRES);
    uint16_t rd_h = VBE_READ(VBE_DISPI_INDEX_YRES);
    uint16_t rd_b = VBE_READ(VBE_DISPI_INDEX_BPP);
    uint16_t rd_e = VBE_READ(VBE_DISPI_INDEX_ENABLE);
	/*
		Optionally debug it.
		we may have an idea why the mode aint working
	*/
	#ifdef DEBUG
    print_str("[VBoxSVGA] Latched: W="); print_hex16(rd_w);
    print_str(" H="); print_hex16(rd_h);
    print_str(" BPP="); print_hex16(rd_b);
    print_str(" EN="); print_hex16(rd_e); print_str("\n");
	#endif
	/*
		Enable
	*/
    if (rd_w != 1024 || rd_h != 768 || rd_b != 32 || !(rd_e & VBE_DISPI_ENABLED)) {
        #ifdef DEBUG
        print_str("[VBoxSVGA] Mode set failed\n");
        #endif
        return -1;
    }
	/*
		Map up the frambuffer
	*/
    pitch = rd_w * 4;
    uint32_t fb_size = rd_h * pitch;
    fb_size = (fb_size + 0xFFF) & ~0xFFF;
	/*
		Chosen one
	*/
    uintptr_t fb_virt = 0xE0000000ULL;
    for (uint32_t off=0; off<fb_size; off+=0x1000) {
		/*
			Mapp up
		*/
        map_page(fb_virt + off, fb_phys + off, /*normal PAGE flags*/PAGE_PRESENT | PAGE_WRITABLE);
    }
	/*
		Set the pointer
	*/
    framebuffer = (volatile uint32_t*)fb_virt;

    #ifdef DEBUG
    print_str("[VBoxSVGA] Framebuffer mapped: VA=");
    print_hex32((uint32_t)fb_virt);
    print_str(" PA="); print_hex32((uint32_t)fb_phys);
    print_str(" Size="); print_hex32(fb_size); print_str("\n");
	#endif
	/*
		Adopt the framebuffer
	*/
	svga_adopt_framebuffer((void*)fb_virt, rd_w, rd_h, pitch, 32);

    #ifdef DEBUG
    print_str("[VBoxSVGA] Banner drawn\n");
    #endif
	/*
		DONE!
	*/
    return 0;
}

/*


	Adopting the buffer
	is important for the APIS\s


*/
void svga_adopt_framebuffer(void *fb, int w, int h, int pitch_bytes, int bpp) {
    g_fb = (volatile uint32_t*)fb;
    g_w = w; g_h = h; g_pitch = pitch_bytes; g_bpp = bpp; g_fmt = 1;
}

/*


	mode switcher


*/
static void svga_hw_set_mode(int w, int h, int bpp) {
	/*
		Disable
	*/
    VBE_WRITE(VBE_DISPI_INDEX_ENABLE, 0);
	/*
		Write them
	*/
    VBE_WRITE(VBE_DISPI_INDEX_XRES, (uint16_t)w);
    VBE_WRITE(VBE_DISPI_INDEX_YRES, (uint16_t)h);
    VBE_WRITE(VBE_DISPI_INDEX_BPP,  (uint16_t)bpp);
    VBE_WRITE(VBE_DISPI_INDEX_VIRT_WIDTH,  (uint16_t)w);
    VBE_WRITE(VBE_DISPI_INDEX_VIRT_HEIGHT, (uint16_t)h);
    VBE_WRITE(VBE_DISPI_INDEX_X_OFFSET, 0);
    VBE_WRITE(VBE_DISPI_INDEX_Y_OFFSET, 0);
	/*
		Enable The main 3s
	*/
    VBE_WRITE(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED | VBE_DISPI_NOCLEARMEM);
	/*
		Update the locals
	*/
    g_w = w; g_h = h; g_bpp = bpp; g_pitch = w * (bpp/8);
}

/*


	Simple APIs we gonna expose


*/


/*


	Some information


*/

void svga_query_caps(uint64_t *caps_out, uint32_t *abi_version_out) {
    if (caps_out) *caps_out = 1ULL;
    if (abi_version_out) *abi_version_out = 1U;
}

void svga_list_modes(void *mode_array_out, int max_modes, int *count_out) {
    if (count_out) *count_out = k_modes_count;
    if (!mode_array_out || max_modes <= 0) return;
    int n = (k_modes_count < max_modes) ? k_modes_count : max_modes;
    svga_mode_t *out = (svga_mode_t*)mode_array_out;
    for (int i=0;i<n;i++) out[i] = k_modes[i];
}

void svga_get_mode(void *mode_out) {
    if (!mode_out) return;
    svga_mode_t *m = (svga_mode_t*)mode_out;
    m->id = -1; m->width = g_w; m->height = g_h; m->bpp = g_bpp; m->fmt = g_fmt;
}

void svga_set_mode(int mode_id) {
    const svga_mode_t *sel = NULL;
    for (int i=0;i<k_modes_count;i++){ if (k_modes[i].id == mode_id) { sel = &k_modes[i]; break; } }
    if (!sel) return;
    svga_hw_set_mode(sel->width, sel->height, sel->bpp);
}

void svga_get_framebuffer_info(void **fb_base_out, int *pitch_out, int *mem_model_out, int *bpp_out) {
    if (fb_base_out) *fb_base_out = (void*)(uintptr_t)g_fb;
    if (pitch_out) *pitch_out = g_pitch;
    if (mem_model_out) *mem_model_out = 1;
    if (bpp_out) *bpp_out = g_bpp;
}

/*


	Drawing


*/
void put_px(int x, int y, uint32_t color) {
    if ((unsigned)x >= (unsigned)g_w || (unsigned)y >= (unsigned)g_h) return;
    volatile uint32_t *row = (volatile uint32_t*)((uintptr_t)g_fb + (uintptr_t)y * (uintptr_t)g_pitch);
    row[x] = color;
}

void svga_clear(uint32_t color) {
    for (int y=0; y<g_h; y++) {
        volatile uint32_t *row = (volatile uint32_t*)((uintptr_t)g_fb + (uintptr_t)y * (uintptr_t)g_pitch);
        for (int x=0; x<g_w; x++) row[x] = color;
    }
}

void svga_draw_pixel(int x, int y, uint32_t color) { put_px(x,y,color); }

void svga_draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = (x1>x0)?(x1-x0):(x0-x1);
    int sx = (x0<x1)?1:-1;
    int dy = (y1>y0)?(y0-y1):(y1-y0);
    int sy = (y0<y1)?1:-1;
    int err = dx+dy;
    while (1) {
        put_px(x0,y0,color);
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void svga_draw_rect(int x, int y, int w, int h, uint32_t color) {
    if (w<=0||h<=0) return;
    svga_draw_line(x,y, x+w-1,y, color);
    svga_draw_line(x,y+h-1, x+w-1,y+h-1, color);
    svga_draw_line(x,y, x,y+h-1, color);
    svga_draw_line(x+w-1,y, x+w-1,y+h-1, color);
}

void svga_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (w<=0||h<=0) return;
    for (int yy=y; yy<y+h; yy++) {
        if ((unsigned)yy >= (unsigned)g_h) continue;
        volatile uint32_t *row = (volatile uint32_t*)((uintptr_t)g_fb + (uintptr_t)yy * (uintptr_t)g_pitch);
        for (int xx=x; xx<x+w; xx++) {
            if ((unsigned)xx < (unsigned)g_w) row[xx] = color;
        }
    }
}

void svga_draw_circle(int cx, int cy, int r, uint32_t color) {
    int x = -r, y = 0, err = 2-2*r;
    do {
        put_px(cx-x, cy+y, color);
        put_px(cx-y, cy-x, color);
        put_px(cx+x, cy-y, color);
        put_px(cx+y, cy+x, color);
        int e2 = err;
        if (e2 <= y) { err += ++y*2+1; }
        if (e2 > x || err > y) { err += ++x*2+1; }
    } while (x < 0);
}

void svga_fill_circle(int cx, int cy, int r, uint32_t color) {
    int x = -r, y = 0, err = 2-2*r;
    do {
        for (int xx=cx+x; xx<=cx-x; xx++) {
            put_px(xx, cy+y, color);
            put_px(xx, cy-y, color);
        }
        int e2 = err;
        if (e2 <= y) { err += ++y*2+1; }
        if (e2 > x || err > y) { err += ++x*2+1; }
    } while (x < 0);
}

void svga_draw_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t c){
    svga_draw_line(x0,y0,x1,y1,c);
    svga_draw_line(x1,y1,x2,y2,c);
    svga_draw_line(x2,y2,x0,y0,c);
}

void svga_fill_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t c){
    if (y1<y0){int t=x0;x0=x1;x1=t; t=y0;y0=y1;y1=t;}
    if (y2<y0){int t=x0;x0=x2;x2=t; t=y0;y0=y2;y2=t;}
    if (y2<y1){int t=x1;x1=x2;x2=t; t=y1;y1=y2;y2=t;}
    int total_height = y2 - y0;
    if (total_height<=0) return;
    for (int i=0;i<=total_height;i++){
        int second_half = (i > y1 - y0) || (y1==y0);
        int segment_height = second_half ? (y2 - y1) : (y1 - y0);
        if (segment_height<=0) continue;
        float alpha = (float)i / (float)total_height;
        float beta  = (float)(i - (second_half ? (y1 - y0) : 0)) / (float)segment_height;
        int Ax = x0 + (int)((x2 - x0) * alpha);
        int Bx = second_half ? (x1 + (int)((x2 - x1) * beta))
                             : (x0 + (int)((x1 - x0) * beta));
        if (Ax > Bx) { int t=Ax; Ax=Bx; Bx=t; }
        int y = y0 + i;
        for (int x=Ax; x<=Bx; x++) put_px(x,y,c);
    }
}

/*


	Hook counter parts
	fixed up the return


*/

/*
	Modes and stuff
*/

void hook_gfx_query_caps(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    uint64_t caps=0; uint32_t abi=0;
    svga_query_caps(&caps,&abi);
    d->args[0] = caps;
    d->args[1] = abi;
}

void hook_gfx_list_modes(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    void *arr = (void*)(uintptr_t)d->args[0];
    int max_modes = (int)d->args[1];
    int count=0;
    svga_list_modes(arr,max_modes,&count);
    d->args[2] = count;
}

void hook_gfx_get_mode(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    void *out = (void*)(uintptr_t)d->args[0];
    svga_get_mode(out);
}

void hook_gfx_set_mode(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    int id = (int)d->args[0];
    svga_set_mode(id);
}

void hook_gfx_get_framebuffer_info(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    void *fb; int pitch, mm, bpp;
    svga_get_framebuffer_info(&fb,&pitch,&mm,&bpp);
    d->args[0] = (uint64_t)(uintptr_t)fb;
    d->args[1] = pitch;
    d->args[2] = mm;
    d->args[3] = bpp;
}

/*
	Hooks for Main drawing
*/

void hook_gfx_clear(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_clear((uint32_t)d->args[0]);
}

void hook_gfx_draw_pixel(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_draw_pixel((int)d->args[0], (int)d->args[1], (uint32_t)d->args[2]);
}

void hook_gfx_draw_line(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_draw_line((int)d->args[0],(int)d->args[1],(int)d->args[2],(int)d->args[3],(uint32_t)d->args[4]);
}

void hook_gfx_draw_rect(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_draw_rect((int)d->args[0],(int)d->args[1],(int)d->args[2],(int)d->args[3],(uint32_t)d->args[4]);
}

void hook_gfx_fill_rect(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_fill_rect((int)d->args[0],(int)d->args[1],(int)d->args[2],(int)d->args[3],(uint32_t)d->args[4]);
}

void hook_gfx_draw_circle(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_draw_circle((int)d->args[0],(int)d->args[1],(int)d->args[2],(uint32_t)d->args[3]);
}

void hook_gfx_fill_circle(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_fill_circle((int)d->args[0],(int)d->args[1],(int)d->args[2],(uint32_t)d->args[3]);
}

void hook_gfx_draw_triangle(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_draw_triangle((int)d->args[0],(int)d->args[1],(int)d->args[2],(int)d->args[3],(int)d->args[4],(int)d->args[5],(uint32_t)d->args[6]);
}

void hook_gfx_fill_triangle(void *data) {
    struct hook_data *d = (struct hook_data*)data;
    svga_fill_triangle((int)d->args[0],(int)d->args[1],(int)d->args[2],(int)d->args[3],(int)d->args[4],(int)d->args[5],(uint32_t)d->args[6]);
}

/*


	Main driver entry point


*/

void _start(void) {
	/*
		Wait for completion
	*/
	t_sleep(2000);
	/*
		Main init the driver
	*/
	vboxsvga_init();
	/*
		Register the hooks
	*/
	reg_hook("gfx_query_caps", hook_gfx_query_caps);
    reg_hook("gfx_list_modes", hook_gfx_list_modes);
    reg_hook("gfx_get_mode", hook_gfx_get_mode);
    reg_hook("gfx_set_mode", hook_gfx_set_mode);
    reg_hook("gfx_get_framebuffer_info", hook_gfx_get_framebuffer_info);
    reg_hook("gfx_clear", hook_gfx_clear);
    reg_hook("gfx_draw_pixel", hook_gfx_draw_pixel);
    reg_hook("gfx_draw_line", hook_gfx_draw_line);
    reg_hook("gfx_draw_rect", hook_gfx_draw_rect);
    reg_hook("gfx_fill_rect", hook_gfx_fill_rect);
    reg_hook("gfx_draw_circle", hook_gfx_draw_circle);
    reg_hook("gfx_fill_circle", hook_gfx_fill_circle);
    reg_hook("gfx_draw_triangle", hook_gfx_draw_triangle);
    reg_hook("gfx_fill_triangle", hook_gfx_fill_triangle);
	/*
		After done...
		Block it as we don't
		want to scedule it after...
	*/
	t_block();
	#ifdef DEBUG
	/*
		Inacse we reach here for some reason
		prolly broken threading
	*/
	print("BLOCK IS SOOOOO STUID");
	#endif
}