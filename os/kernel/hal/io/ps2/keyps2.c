/*
	Header files
*/
#include "keyps2.h"
/*
	Declaring
*/
keyboard_state_t keyboard_state = {0};
keyboard_buffer_t input_buffer = {0};
bool extended_scancode = false; // Init as 0
/*
	main init
*/
void ps2_keyboard_init(uint8_t port) {
    #ifdef DEBUG
    printf("PS2: Initializing keyboard on port %d\n", port);
    #endif
    uint8_t pic_mask = inb(0x21);  // Read current PIC1 mask
    pic_mask &= ~0x02;             // Clear bit 1 (IRQ1 - keyboard)
    outb(0x21, pic_mask);          // Write back the mask
    /*
		Reset
	*/
    if (ps2_send_device_command(port, PS2_DEV_RESET)) {
        uint8_t response;
        if (ps2_read_data_timeout(&response, 1000)) {
            if (response == PS2_RESP_SELF_TEST_OK) {
                #ifdef DEBUG
                printf("PS2: Keyboard self-test passed\n");
                #endif
            } else {
                #ifdef DEBUG
                printf("PS2: Keyboard self-test failed: 0x%02X\n", response);
                #endif
            }
        }
    }
    
	/*
		Set to 1
	*/
    ps2_send_device_command_with_data(port, PS2_DEV_SET_SCAN_SET, 1);
    keyboard_state.num_lock = true;
    ps2_send_device_command(port, PS2_DEV_ENABLE_SCAN/*enable scan*/); 
    
    /*
		INIT THE BUFFER!
	*/
    input_buffer.head = 0;
    input_buffer.tail = 0;
    input_buffer.count = 0;
    
    #ifdef DEBUG
    printf("PS2: Keyboard initialization complete\n");
    #endif
}
/*
	SOme buffer handling
	as well as the scancode
*/
char scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) return 0;
    
    char ascii = 0;
    bool use_shift = keyboard_state.shift_pressed;
    
	/*
		We need to handle cap lock
		idk why but my stupid mind i wanted to try to just
		and lock the shft key so the shifthandler handles automatically but
		we only want the normal alpha keys only to be affected
	*/
    if (keyboard_state.caps_lock) {
        char normal = scancode_to_ascii_normal[scancode];
        if (normal >= 'a' && normal <= 'z') {
            use_shift = !use_shift; /*SHIFT*/
        }
    }
    
    /*
		Fetch the stuff
	*/
    if (use_shift) {
        ascii = scancode_to_ascii_shift[scancode];
    } else {
        ascii = scancode_to_ascii_normal[scancode];
    }
    
    return ascii;
}
/*
	Some input butter
*/
void keyboard_buffer_put(char c) {
    if (input_buffer.count < KEYBOARD_BUFFER_SIZE) {
        input_buffer.buffer[input_buffer.head] = c;
        input_buffer.head = (input_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
        input_buffer.count++;
    }
}

char keyboard_buffer_get(void) {
    if (input_buffer.count > 0) {
        char c = input_buffer.buffer[input_buffer.tail];
        input_buffer.tail = (input_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
        input_buffer.count--;
        return c;
    }
    return 0;
}

bool keyboard_buffer_empty(void) {
    return input_buffer.count == 0;
}

int keyboard_buffer_available(void) { /*probably spelled wrong or did i????*/
    return input_buffer.count;
}
/*
	Handle others
*/
void handle_special_key(uint8_t scancode, bool key_pressed) {
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            keyboard_state.shift_pressed = key_pressed;
            break;
            
        case KEY_LCTRL:
            keyboard_state.ctrl_pressed = key_pressed;
            break;
            
        case KEY_LALT:
            keyboard_state.alt_pressed = key_pressed;
            break;
            
        case KEY_CAPS_LOCK:
            if (key_pressed) {
                keyboard_state.caps_lock = !keyboard_state.caps_lock;
            }
            break;
            
        case KEY_NUM_LOCK:
            if (key_pressed) {
                keyboard_state.num_lock = !keyboard_state.num_lock;
            }
            break;
            
        case KEY_SCROLL_LOCK:
            if (key_pressed) {
                keyboard_state.scroll_lock = !keyboard_state.scroll_lock;
            }
            break;
            
        case KEY_F1:
        case KEY_F2:
        case KEY_F3:
        case KEY_F4:
        case KEY_F5:
        case KEY_F6:
        case KEY_F7:
        case KEY_F8:
        case KEY_F9:
        case KEY_F10:
        case KEY_F11:
        case KEY_F12:
            if (key_pressed) {
				/* // FOR NOW IGNORE as this doesnt work
                keyboard_state.function_key = scancode - KEY_F1 + 1;
                #ifdef DEBUG
                printf("Function key F%d pressed\n", keyboard_state.function_key);
                #endif
				*/ 
            }
            break;
    }
}

/*
	Handle extended scancodes (0xE0 ones)
*/
void handle_extended_scancode(uint8_t scancode, bool key_pressed) {
	/*
		Simpel switch case works
	*/
    switch (scancode) {
        case 0x1D: // Right Ctrl
            keyboard_state.ctrl_pressed = key_pressed;
            break;
            
        case 0x38: // Right Alt
            keyboard_state.alt_pressed = key_pressed;
            break;
            
        case 0x47: // Home
            if (key_pressed) keyboard_buffer_put('\x01'); // Ctrl+A equivalent
            break;
            
        case 0x48: // Up Arrow
            if (key_pressed) keyboard_buffer_put('\x1B'); // ESC sequence start
            break;
            
        case 0x49: // Page Up
            if (key_pressed) printf("Page Up pressed\n");
            break;
            
        case 0x4B: // Left Arrow
            if (key_pressed) keyboard_buffer_put('\x02'); // Ctrl+B equivalent
            break;
            
        case 0x4D: // Right Arrow
            if (key_pressed) keyboard_buffer_put('\x06'); // Ctrl+F equivalent
            break;
            
        case 0x4F: // End
            if (key_pressed) keyboard_buffer_put('\x05'); // Ctrl+E equivalent
            break;
            
        case 0x50: // Down Arrow
            if (key_pressed) keyboard_buffer_put('\x0E'); // Ctrl+N equivalent
            break;
            
        case 0x51: // Page Down
            if (key_pressed) printf("Page Down pressed\n");
            break;
            
        case 0x52: // Insert
            if (key_pressed) printf("Insert pressed\n");
            break;
            
        case 0x53: // Delete
            if (key_pressed) keyboard_buffer_put('\x7F'); // DEL character
            break;
            
        default:
            #ifdef DEBUG
            printf("Unknown extended scancode: 0x%02X\n", scancode);
            #endif
            break;
    }
}

/*
	Main keyboard interrupt handler very important and may wire up to the irq
*/
void ps2_keyboard_interrupt_handler(interrupt_frame_t* frame) {
    uint8_t scancode = inb(PS2_DATA_PORT);
    /*
		handle extension
	*/
    if (scancode == 0xE0) {
        extended_scancode = true;
        return;
    }
    
    /*
		is it pressed?
		(thats what she said)
	*/
    bool key_pressed = !(scancode & 0x80);
    uint8_t key = scancode & 0x7F;
    
    /*
		We must update the last scancode
	*/
    keyboard_state.last_scancode = scancode;
    
    /*
		handle the extension
	*/
    if (extended_scancode) {
        handle_extended_scancode(key, key_pressed);
        extended_scancode = false;
        return;
    }
    
    /*
		Handle the modifiers
	*/
    handle_special_key(key, key_pressed);
    
    /*
		Also handle ASCII
	*/
    if (key_pressed) {
        char ascii = scancode_to_ascii(key);
        
        if (ascii != 0) {
            /*
				HANDLE comb
			*/
            if (keyboard_state.ctrl_pressed && ascii >= 'a' && ascii <= 'z') {
                ascii = ascii - 'a' + 1; // Convert to Ctrl+letter
            } else if (keyboard_state.ctrl_pressed && ascii >= 'A' && ascii <= 'Z') {
                ascii = ascii - 'A' + 1; // Convert to Ctrl+letter
            }
            
            keyboard_buffer_put(ascii);
        }
    }
}

/*


	SOME API for public


*/
char getchar(void) {
    return keyboard_buffer_get();
}

char getchar_blocking(void) {
    while (keyboard_buffer_empty()) {
        __asm__ volatile("nop");
    }
    return keyboard_buffer_get();
}

bool kbhit(void) {
    return !keyboard_buffer_empty();
}

int checkbuffer(void) {
    return keyboard_buffer_available();
}

void kbflush(void) {
    input_buffer.head = 0;
    input_buffer.tail = 0;
    input_buffer.count = 0;
}

keyboard_state_t* get_kbstate(void) {
    return &keyboard_state;
}

bool ps2_has_keyboard(void) {
    return ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_OLD ||
           ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_MF2;
}