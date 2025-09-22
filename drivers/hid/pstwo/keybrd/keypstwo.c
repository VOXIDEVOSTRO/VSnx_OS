/*


	Super simple PS2 keyboard driver....
	simple enough to work in a single file


*/
/*
	Header files
*/
#include "keyps2.h"
/*
	For the hooks!
*/
#include "../../../../os/kernel/driver/hookcall/hookreg.h"
#include "../../../../os/kernel/driver/hook.h"
#include "../../../../os/kernel/syscalls/func.h"
/*
	Some global declaration
*/
ps2_controller_t ps2_controller = {0};
/*
	Some I/O ports
	(just feeling lazy and copied the code from kernel/hal)
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
    Low-level PS/2 func
	because we need it
*/
/*
	SOME waiting
*/
bool ps2_wait_input(void) {
    uint32_t timeout = 100000;
	/*
		Timeout just to avoid hanging
	*/
    while (timeout--) {
        if (!(inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL)) {
            return true;
        }
    }
    return false;
}

bool ps2_wait_output(void) {
    uint32_t timeout = 100000;
	/*
		Same here
	*/
    while (timeout--) {
        if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            return true;
        }
    }
    return false;
}

/*
	Some PS2 command and data
*/

void ps2_write_command(uint8_t command) {
    ps2_wait_input();
    outb(PS2_COMMAND_PORT, command);
}

void ps2_write_data(uint8_t data) {
    ps2_wait_input();
    outb(PS2_DATA_PORT, data);
}

uint8_t ps2_read_data(void) {
    ps2_wait_output();
    return inb(PS2_DATA_PORT);
}

bool ps2_read_data_timeout(uint8_t* data, uint32_t timeout_ms) {
    uint32_t timeout = timeout_ms * 1000;
    while (timeout--) {
        if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            *data = inb(PS2_DATA_PORT);
            return true;
        }
    }
    return false;
}

/*
    Some communication
*/
bool ps2_send_device_command(uint8_t port, uint8_t command) {
    if (port == 2) {
        ps2_write_command(PS2_CMD_WRITE_PORT2);
    }
    ps2_write_data(command);
    uint8_t response;
    if (ps2_read_data_timeout(&response, 100)) {
        return response == PS2_RESP_ACK;
    }
    return false;
}

bool ps2_send_device_command_with_data(uint8_t port, uint8_t command, uint8_t data/*Just writes with data too*/) {
    if (!ps2_send_device_command(port, command)) {
        return false;
    }
    if (port == 2) {
        ps2_write_command(PS2_CMD_WRITE_PORT2);
    }
    ps2_write_data(data);

    uint8_t response;
    if (ps2_read_data_timeout(&response, 100)) {
        return response == PS2_RESP_ACK;
    }
    return false;
}

/*
    And detection
*/
ps2_device_type_t ps2_identify_device(uint8_t port/*simple idetification for mouse and keyboard*/) {

    // Disable scanning
    if (!ps2_send_device_command(port, PS2_DEV_DISABLE_SCAN)) {
        return PS2_DEVICE_NONE;
    }

    /*
		IDENTIFY YOURSELF
	*/
    if (port == 2) {
        ps2_write_command(PS2_CMD_WRITE_PORT2);
    }
    ps2_write_data(PS2_DEV_IDENTIFY);

    uint8_t response;
    if (!ps2_read_data_timeout(&response, 100)) {
        
        return PS2_DEVICE_NONE;
    }

    if (response != PS2_RESP_ACK) {

        return PS2_DEVICE_NONE;
    }

    /*
		read the id
	*/
    uint8_t id_bytes[2] = {0};
    int id_count = 0;

    /*
		Check both bytes
	*/
    if (ps2_read_data_timeout(&id_bytes[0], 100)) {
        id_count = 1;
        if (ps2_read_data_timeout(&id_bytes[1], 100)) {
            id_count = 2;
        }
    }
    ps2_device_type_t device_type = PS2_DEVICE_NONE;

    if (id_count == 0) {
		/*
			OLD PS2 keyboard
		*/
        device_type = PS2_DEVICE_KEYBOARD_OLD;

    } else if (id_count == 2) {
        if (id_bytes[0] == 0xAB && id_bytes[1] == 0x83) {
			/*
				More modern Keyboard
			*/
            device_type = PS2_DEVICE_KEYBOARD_MF2;
        
        } else if (id_bytes[0] == 0xAB && id_bytes[1] == 0x41) {
			/*
				Translation Keyboard but modern
			*/
            device_type = PS2_DEVICE_KEYBOARD_MF2;
            
        } else if (id_bytes[0] == 0x00) {
			/*
				Standard Mouse
			*/
            device_type = PS2_DEVICE_MOUSE_STANDARD;
            
        } else if (id_bytes[0] == 0x03) {
			/*
				Modern mouse but still standard with wheel
			*/
            device_type = PS2_DEVICE_MOUSE_WHEEL;
    
        } else if (id_bytes[0] == 0x04) {
			/*
				5 buttoned Mouse
			*/
            device_type = PS2_DEVICE_MOUSE_5BUTTON;
         
        }
    }
	/*
		re-enable
	*/
    ps2_send_device_command(port, PS2_DEV_ENABLE_SCAN);

    return device_type;
}
/*
	Some teeny tiny init check
*/
bool ps2_is_initialized(void) {
    return ps2_controller.initialized;
}
/*


	PS2 keyboard


*/
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
	/*
		Unmask
		For the irq1
	*/
    uint8_t pic_mask = inb(0x21);
    pic_mask &= ~0x02;             
    outb(0x21, pic_mask);
    /*
		Reset
	*/
    if (ps2_send_device_command(port, PS2_DEV_RESET)) {
        uint8_t response;
        if (ps2_read_data_timeout(&response, 1000)) {
            if (response == PS2_RESP_SELF_TEST_OK) {
				print("Test was OK");
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
}
/*
	Some buffer handling
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
	like putchar?
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
/*
	Simple buffer clarification
*/
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
		/*
			Left shift
			main shift lets say
		*/
        case KEY_LSHIFT:
		/*
			Right shift
		*/
        case KEY_RSHIFT:
            keyboard_state.shift_pressed = key_pressed;
            break;
		/*
			main left ctrl
		*/
        case KEY_LCTRL:
            keyboard_state.ctrl_pressed = key_pressed;
            break;
		/*
			Main left alt
		*/
        case KEY_LALT:
            keyboard_state.alt_pressed = key_pressed;
            break;
		/*
			Caps lock
		*/
        case KEY_CAPS_LOCK:
            if (key_pressed) {
                keyboard_state.caps_lock = !keyboard_state.caps_lock;
            }
            break;
		/*
			Num lock
		*/
        case KEY_NUM_LOCK:
            if (key_pressed) {
                keyboard_state.num_lock = !keyboard_state.num_lock;
            }
            break;
		/*
			Scroll lock
		*/
        case KEY_SCROLL_LOCK:
            if (key_pressed) {
                keyboard_state.scroll_lock = !keyboard_state.scroll_lock;
            }
            break;
		/*
			Unused function keys
			or unimplimented
		*/
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
			/*
				For debug...
			*/
            if (key_pressed) print("Page Up pressed\n");
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
			/*
				For debug...
			*/
            if (key_pressed) print("Page Down pressed\n");
            break;

        case 0x52: // Insert
            if (key_pressed) print("Insert pressed\n");
            break;

        case 0x53: // Delete
            if (key_pressed) keyboard_buffer_put('\x7F'); // DEL character
            break;

        default:
			/*
				Nothing to see here
			*/            
            break;
    }
}

/*
	Main keyboard interrupt handler very important and may wire up to the hooks
*/
void ps2_keyboard_interrupt_handler(interrupt_frame_t* frame) {
	/*
		Get the scancode
	*/

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
			/*
				Parse up
			*/
            keyboard_buffer_put(ascii);
        }
    }
}

/*


	SOME API for public


*/

char getchar(void) {
	/*
		Non blocking
	*/
    return keyboard_buffer_get();
}

char getchar_blocking(void) {
	/*
		Blocking
	*/
    while (keyboard_buffer_empty()) {
        __asm__ volatile("nop");
    }
    return keyboard_buffer_get();
}

bool kbhit(void) {
	/*
		anything there?
	*/
    return !keyboard_buffer_empty();
}

int checkbuffer(void) {
	/*
		Set at init
	*/
    return keyboard_buffer_available();
}

void kbflush(void) {
	/*
		Just clear up
	*/
    input_buffer.head = 0;
    input_buffer.tail = 0;
    input_buffer.count = 0;
}

keyboard_state_t* get_kbstate(void) {
	/*
		Duh...
	*/
    return &keyboard_state;
}

bool ps2_has_keyboard(void) {
	/*
		Can be both
	*/
    return /*
				The old PS2 keyboard
		   */
		   ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_OLD ||
		   /*
		 		Or more modern one  
		   */
           ps2_controller.port1_device == PS2_DEVICE_KEYBOARD_MF2;
}

/*
	Hook wrappers for the hooks
*/

void ps2_hook_init(void *data) {

    ps2_keyboard_init(1);
}

void ps2_hook_shutdown(void *data) {
	/*
		Ignore...
	*/
}

void ps2_hook_has_key(void *data) {

    struct hook_data *d = (struct hook_data*)data;
    uint32_t has_key = !keyboard_buffer_empty();
    d->args[0] = has_key;
}

void ps2_hook_get_key(void *data) {

    struct hook_data *d = (struct hook_data*)data;
    if (!keyboard_buffer_empty()) {
        d->args[0] = keyboard_buffer_get();
    } else {
        d->args[0] = 0;
    }
}

void ps2_hook_query_state(void *data) {

    // TODO: Implement key state query if needed
    struct hook_data *d = (struct hook_data*)data;
    d->args[0] = 0;
}

void ps2_hook_set_leds(void *data) {
	/*
		IGNORE
	*/
}

void ps2_hook_get_modifiers(void *data) {

    struct hook_data *d = (struct hook_data*)data;
    uint32_t mods = 0;
    if (keyboard_state.shift_pressed) mods |= 1 << 0;
    if (keyboard_state.ctrl_pressed) mods |= 1 << 1;
    if (keyboard_state.alt_pressed) mods |= 1 << 2;
    d->args[0] = mods;
}

void ps2_hook_translate(void *data) {
	/*
		Later we can add translation...
	*/
    struct hook_data *d = (struct hook_data*)data;
    d->args[0] = d->args[0]; // Pass through
}

void ps2_hook_interrupt_handler(void *data) {
	/*
		MAIN interrupt main point!
	*/
    struct hook_data *d = (struct hook_data*)data;
    interrupt_frame_t* frame = (interrupt_frame_t*)d->args[0];
    ps2_keyboard_interrupt_handler(frame);
}

/*
	Driver start point.
	Just the simple init for the driver
*/
void _start(void) {
	/*
		Init da driver
	*/
	ps2_keyboard_init(1/*At port 1*/);
	/*
		Hooks register
	*/
    reg_hook("kbd_init", ps2_hook_init);
    reg_hook("kbd_shutdown", ps2_hook_shutdown);
    reg_hook("kbd_has_key", ps2_hook_has_key);
    reg_hook("kbd_get_key", ps2_hook_get_key);
    reg_hook("kbd_query_state", ps2_hook_query_state);
    reg_hook("kbd_set_leds", ps2_hook_set_leds);
    reg_hook("kbd_get_modifiers", ps2_hook_get_modifiers);
    reg_hook("kbd_translate", ps2_hook_translate);
    reg_hook("kbd_interrupt_handler", ps2_hook_interrupt_handler);
	/*
		save up resourcses...
	*/
	t_block();
	print("BLOCK IS SOOOOO STUID");
}
