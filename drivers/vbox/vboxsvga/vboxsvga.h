/*
	This is also just VBE btw...
*/
#ifndef VBE_H
#define VBE_H
/*
	Heads and tails
*/
#include "../../pcibus/pci.h"
/*
	Hooks and stuff. basic for most drivers
*/
#include "../../../os/kernel/driver/hook.h"
#include "../../../os/kernel/driver/hookcall/hookreg.h"
/*
	Some syscalls... Probably not needed... but just for debug...
*/
#include "../../../os/kernel/syscalls/func.h"

/*

	These port index were the most annoying.
	because me missing VMSVGA with VBOXSVGA...
	i guess this fixes it

*/
#define VBE_DISPI_IOPORT_INDEX   0x1CE
#define VBE_DISPI_IOPORT_DATA    0x1CF
/*
	Simple enums for DISPI Index
*/
enum {
	/*
		Index: ID :- 0x0
	*/
    VBE_DISPI_INDEX_ID          = 0x0,
	/*
		Index: X-Resol :- 0x1
	*/
    VBE_DISPI_INDEX_XRES        = 0x1,
	/*
		Index: Y-Resol :- 0x2
	*/
    VBE_DISPI_INDEX_YRES        = 0x2,
	/*
		Index: BPP :- 0x3
	*/
    VBE_DISPI_INDEX_BPP         = 0x3,
	/*
		Index: toggle enable :- 0x4
	*/
    VBE_DISPI_INDEX_ENABLE      = 0x4,
	/*
		Index: Bank :- 0x5
	*/
    VBE_DISPI_INDEX_BANK        = 0x5,
	/*
		Virtual width index
	*/
    VBE_DISPI_INDEX_VIRT_WIDTH  = 0x6,
	/*
		Virtual height index
	*/
    VBE_DISPI_INDEX_VIRT_HEIGHT = 0x7,
	/*
		Index: X-off = 8
	*/
    VBE_DISPI_INDEX_X_OFFSET    = 0x8,
	/*
		Index: Y-off = 9
	*/
    VBE_DISPI_INDEX_Y_OFFSET    = 0x9,
	/*
		CFG/Configouration(spelled it wrong prolly) index :-
		= 10 or A
	*/
    VBE_DISPI_INDEX_CFG         = 0xA
};
/*
	Some extras one of them
*/
#define VBE_DISPI_ENABLED        0x0001
/*
	We need LFB
*/
#define VBE_DISPI_LFB_ENABLED    0x0040
#define VBE_DISPI_NOCLEARMEM     0x0080
/*
	Some helpers for wrting to the device
*/
static inline void VBE_WRITE(uint16_t index, uint16_t value) {
	/*
		I also mixed this up...
	*/
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}
static inline uint16_t VBE_READ(uint16_t index) {
	/*
		As well as this...
	*/
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}
/*
	Some API helpers
	Because we had some hook return problems
	This because we need the PCI driver to be there too...
*/
#define U64P(p)   ((uint64_t)(uintptr_t)(p))
#define P64(u)    ((void*)(uintptr_t)(u))
/*
	Main return pointer
*/
#define RET_PTR(r) ((void*)(uintptr_t)(r))
/*
	A wrapper to handle the PCI calls
*/
static inline int64_t hook_call_ret(const char *name, struct hook_data *d) {
    hook_func_t f/*function*/ = get_hookling(name);
    return f/*function_ret*/ ? f((void*)d)/*NULL*/ : -1; /*No hook*/
}
/*
	extremely simple mode descriptor and may work for now
*/
typedef struct svga_mode {
    int id;
	/*
		Normal display
	*/
    int width;
    int height;
	/*
		WE MAY fix this...
		i did it because i am getting some color issues...
	*/
    int bpp/*32*/;
    int fmt/*This is R8G8B8 format probably*/;
} svga_mode_t;
/*
	Prototypes
*/
#ifndef VBE_PROT
#define VBE_PROT
void print_num(uint32_t num);
void print_hex_num(uint32_t num, int digits);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);
pci_device_t *pci_device_create(uint8_t bus, uint8_t device, uint8_t function);
void pci_device_destroy(pci_device_t *device);
void pci_device_read_config(pci_device_t *device);
pci_bus_t *pci_bus_create(uint8_t bus_number);
void pci_bus_destroy(pci_bus_t *bus);
void pci_bus_scan_devices(pci_bus_t *bus);
void pci_enumerate(void);
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t *pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
pci_device_t *pci_get_devices(void);
uint64_t pci_get_bar_address(pci_device_t *device, int bar_index);
uint64_t pci_get_bar_size(pci_device_t *device, int bar_index);
_Bool pci_bar_is_io(pci_device_t *device, int bar_index);
_Bool pci_bar_is_64bit(pci_device_t *device, int bar_index);
void pci_enable_bus_mastering(pci_device_t *device);
void pci_disable_bus_mastering(pci_device_t *device);
void pci_enable_io_space(pci_device_t *device);
void pci_disable_io_space(pci_device_t *device);
void pci_enable_memory_space(pci_device_t *device);
void pci_disable_memory_space(pci_device_t *device);
void pci_enable_interrupts(pci_device_t *device);
void pci_disable_interrupts(pci_device_t *device);
int vboxsvga_init(void);
void svga_adopt_framebuffer(void *fb, int w, int h, int pitch_bytes, int bpp);
void svga_query_caps(uint64_t *caps_out, uint32_t *abi_version_out);
void svga_list_modes(void *mode_array_out, int max_modes, int *count_out);
void svga_get_mode(void *mode_out);
void svga_set_mode(int mode_id);   
void svga_get_framebuffer_info(void **fb_base_out, int *pitch_out, int *mem_model_out, int *bpp_out);
void svga_clear(uint32_t color);
void svga_draw_pixel(int x, int y, uint32_t color);
void svga_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void svga_draw_rect(int x, int y, int w, int h, uint32_t color);
void svga_fill_rect(int x, int y, int w, int h, uint32_t color);
void svga_draw_circle(int cx, int cy, int r, uint32_t color);
void svga_fill_circle(int cx, int cy, int r, uint32_t color);
void svga_draw_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t c);
void svga_fill_triangle(int x0,int y0,int x1,int y1,int x2,int y2,uint32_t c);
#endif
#endif