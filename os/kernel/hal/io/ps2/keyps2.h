#ifndef PS2KEY_H
#define PS2KEY_H
/*
	Gloabls
*/
#include "ps2.h"
#include "../../interrupts/interrupts.h"
/*
	Sturctures
*/
typedef struct {
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;
    uint8_t last_scancode;
} keyboard_state_t;
/*
	Also the scancodes FOR SET1
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
void ps2_keyboard_init(uint8_t port);
char scancode_to_ascii(uint8_t scancode);
void keyboard_buffer_put(char c);
char keyboard_buffer_get(void);
bool keyboard_buffer_empty(void);
int keyboard_buffer_available(void);
void handle_special_key(uint8_t scancode, _Bool key_pressed);
void handle_extended_scancode(uint8_t scancode, _Bool key_pressed);
void ps2_keyboard_interrupt_handler(interrupt_frame_t *frame);
char getchar(void);
char getchar_blocking(void);
bool kbhit(void);
int checkbuffer(void);
void kbflush(void);
keyboard_state_t *get_kbstate(void);
bool ps2_has_keyboard(void);
#endif