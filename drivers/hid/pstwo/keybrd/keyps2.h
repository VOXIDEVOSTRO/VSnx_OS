#ifndef PS2KEY_H
#define PS2KEY_H
/*
	headerz
*/
#include "../ps2.h"
#include "../../../../os/kernel/hal/interrupts/interrupts.h"
/*
	Sturctures
*/
typedef struct {
	/*
		Shift modifer
	*/
    bool shift_pressed;
	/*
		ctrl modifier
	*/
    bool ctrl_pressed;
	/*
		alt modifer
	*/
    bool alt_pressed;
	/*
		lock keys
	*/
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
	/*
		Last Scancode
	*/
    uint8_t last_scancode;
} keyboard_state_t;
/*
	scan code for set 1

	This would be helpful

	| Scan Code                     | Key or action                    |
	|-------------------------------|----------------------------------|
	| 0x01                          | escape pressed                   |
	| 0x02                          | 1 pressed                        |
	| 0x03                          | 2 pressed                        |
	| 0x04                          | 3 pressed                        |
	| 0x05                          | 4 pressed                        |
	| 0x06                          | 5 pressed                        |
	| 0x07                          | 6 pressed                        |
	| 0x08                          | 7 pressed                        |
	| 0x09                          | 8 pressed                        |
	| 0x0A                          | 9 pressed                        |
	| 0x0B                          | 0 (zero) pressed                 |
	| 0x0C                          | - pressed                        |
	| 0x0D                          | = pressed                        |
	| 0x0E                          | backspace pressed                |
	| 0x0F                          | tab pressed                      |
	| 0x10                          | Q pressed                        |
	| 0x11                          | W pressed                        |
	| 0x12                          | E pressed                        |
	| 0x13                          | R pressed                        |
	| 0x14                          | T pressed                        |
	| 0x15                          | Y pressed                        |
	| 0x16                          | U pressed                        |
	| 0x17                          | I pressed                        |
	| 0x18                          | O pressed                        |
	| 0x19                          | P pressed                        |
	| 0x1A                          | [ pressed                        |
	| 0x1B                          | ] pressed                        |
	| 0x1C                          | enter pressed                    |
	| 0x1D                          | left control pressed             |
	| 0x1E                          | A pressed                        |
	| 0x1F                          | S pressed                        |
	| 0x20                          | D pressed                        |
	| 0x21                          | F pressed                        |
	| 0x22                          | G pressed                        |
	| 0x23                          | H pressed                        |
	| 0x24                          | J pressed                        |
	| 0x25                          | K pressed                        |
	| 0x26                          | L pressed                        |
	| 0x27                          | ; pressed                        |
	| 0x28                          | ' (single quote) pressed         |
	| 0x29                          | ` (back tick) pressed            |
	| 0x2A                          | left shift pressed               |
	| 0x2B                          | \ pressed                        |
	| 0x2C                          | Z pressed                        |
	| 0x2D                          | X pressed                        |
	| 0x2E                          | C pressed                        |
	| 0x2F                          | V pressed                        |
	| 0x30                          | B pressed                        |
	| 0x31                          | N pressed                        |
	| 0x32                          | M pressed                        |
	| 0x33                          | , pressed                        |
	| 0x34                          | . pressed                        |
	| 0x35                          | / pressed                        |
	| 0x36                          | right shift pressed              |
	| 0x37                          | (keypad) * pressed               |
	| 0x38                          | left alt pressed                 |
	| 0x39                          | space pressed                    |
	| 0x3A                          | CapsLock pressed                 |
	| 0x3B–0x44                     | F1 to F10 pressed                |
	| 0x45                          | NumberLock pressed               |
	| 0x46                          | ScrollLock pressed               |
	| 0x47–0x53                     | Keypad 7–. pressed               |
	| 0x57–0x58                     | F11, F12 pressed                 |
	| 0x81–0xD8                     | All above keys released          |
	| 0xE0,0x10                     | multimedia: prev track press     |
	| 0xE0,0x19                     | multimedia: next track press     |
	| 0xE0,0x1C                     | keypad enter pressed             |
	| 0xE0,0x1D                     | right control pressed            |
	| 0xE0,0x20                     | multimedia: mute pressed         |
	| 0xE0,0x21                     | multimedia: calculator press     |
	| 0xE0,0x22                     | multimedia: play pressed         |
	| 0xE0,0x24                     | multimedia: stop pressed         |
	| 0xE0,0x2E                     | multimedia: vol down pressed     |
	| 0xE0,0x30                     | multimedia: vol up pressed       |
	| 0xE0,0x32                     | multimedia: WWW home pressed     |
	| 0xE0,0x35                     | keypad / pressed                 |
	| 0xE0,0x38                     | right alt (altGr) pressed        |
	| 0xE0,0x47–0x53                | cursor/home/insert keys          |
	| 0xE0,0x5B–0x5F                | GUI & ACPI keys pressed          |
	| 0xE0,0x65–0x6D                | multimedia web keys              |
	| 0xE0,0x90–0xED                | above multimedia released        |
	| 0xE0,0x2A,0xE0,0x37           | print screen pressed             |
	| 0xE0,0xB7,0xE0,0xAA           | print screen released            |
	| 0xE1,0x1D,0x45,0xE1,0x9D,0xC5 | pause pressed                    |
*/
/*
	Also the scancodes to ascii FOR SET1
	all 128 entries but some may be missing
*/
static const char scancode_to_ascii_normal[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*shifted*/
static const char scancode_to_ascii_shift[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
/*
	Constants for special keys (Scancode Set 1)
*/
#define KEY_ESC         0x01 /*ESC*/
#define KEY_BACKSPACE   0x0E /*Backspace*/
#define KEY_TAB         0x0F /*TAB*/
#define KEY_ENTER       0x1C /*enter could be the left or right*/
#define KEY_LCTRL       0x1D /*left ctrl*/
#define KEY_LSHIFT      0x2A /*left shift*/
#define KEY_RSHIFT      0x36 /*right shift*/
#define KEY_LALT        0x38 /*left alt*/
#define KEY_SPACE       0x39 /*space bar*/
#define KEY_CAPS_LOCK   0x3A /*caps lock*/
#define KEY_F1          0x3B /*function key 1*/
#define KEY_F2          0x3C /*function key 2*/
#define KEY_F3          0x3D /*function key 3*/
#define KEY_F4          0x3E /*function key 4*/
#define KEY_F5          0x3F /*function key 5*/
#define KEY_F6          0x40 /*function key 6*/
#define KEY_F7          0x41 /*function key 7*/
#define KEY_F8          0x42 /*function key 8*/
#define KEY_F9          0x43 /*function key 9*/
#define KEY_F10         0x44 /*function key 10*/
#define KEY_NUM_LOCK    0x45 /*Num lock - do any one uses this???*/
#define KEY_SCROLL_LOCK 0x46 /*sroll lock - anyone???*/
#define KEY_F11         0x57 /*function key 11*/
#define KEY_F12         0x58 /*function key 12*/
/*
	keyboard Buffer
*/
#define KEYBOARD_BUFFER_SIZE 256 
/*
	buffer struct
*/
typedef struct {
    char buffer[KEYBOARD_BUFFER_SIZE/*256*/];
    volatile int head; // start
    volatile int tail; // end
    volatile int count;
} keyboard_buffer_t;
/*
	Buffer globals
*/
extern keyboard_buffer_t input_buffer;
extern bool extended_scancode;
/*
	globals
*/
extern keyboard_state_t keyboard_state;
/*
	prototypes
*/
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
_Bool ps2_wait_input(void);
_Bool ps2_wait_output(void);
void ps2_write_command(uint8_t command);
void ps2_write_data(uint8_t data);
uint8_t ps2_read_data(void);
_Bool ps2_read_data_timeout(uint8_t *data, uint32_t timeout_ms);
_Bool ps2_send_device_command(uint8_t port, uint8_t command);
_Bool ps2_send_device_command_with_data(uint8_t port, uint8_t command, uint8_t data);
ps2_device_type_t ps2_identify_device(uint8_t port);
_Bool ps2_is_initialized(void);
void ps2_keyboard_init(uint8_t port);
char scancode_to_ascii(uint8_t scancode);
void keyboard_buffer_put(char c);
char keyboard_buffer_get(void);
_Bool keyboard_buffer_empty(void);
int keyboard_buffer_available(void);
void handle_special_key(uint8_t scancode, _Bool key_pressed);
void handle_extended_scancode(uint8_t scancode, _Bool key_pressed);
void ps2_keyboard_interrupt_handler(interrupt_frame_t *frame);
char getchar(void);
char getchar_blocking(void);
_Bool kbhit(void);
int checkbuffer(void);
void kbflush(void);
keyboard_state_t *get_kbstate(void);
_Bool ps2_has_keyboard(void);
/*
	Just hook counter parts
*/
void ps2_hook_init(void *data);
void ps2_hook_shutdown(void *data);
void ps2_hook_has_key(void *data);
void ps2_hook_get_key(void *data);
void ps2_hook_query_state(void *data);
void ps2_hook_set_leds(void *data);
void ps2_hook_get_modifiers(void *data);
void ps2_hook_translate(void *data);
#endif