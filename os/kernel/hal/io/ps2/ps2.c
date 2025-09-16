/*
	Header files
*/
#include "ps2.h"
/*
	Some global declaration
*/
ps2_controller_t ps2_controller = {0};

/*
    Low-level PS/2 func
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
    #ifdef DEBUG
    #ifdef DEBUG
    printf("PS2: Identifying device on port %d\n", port);
    #endif
    #endif
    
    // Disable scanning
    if (!ps2_send_device_command(port, PS2_DEV_DISABLE_SCAN)) {
        #ifdef DEBUG
        #ifdef DEBUG
        printf("PS2: Failed to disable scanning on port %d\n", port);
        #endif
        #endif
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
        #ifdef DEBUG
        #ifdef DEBUG
        printf("PS2: No response to identify command on port %d\n", port);
        #endif
        #endif
        return PS2_DEVICE_NONE;
    }
    
    if (response != PS2_RESP_ACK) {
        #ifdef DEBUG
        #ifdef DEBUG
        printf("PS2: Device on port %d did not acknowledge identify command\n", port);
        #endif
        #endif
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
    
    #ifdef DEBUG
    #ifdef DEBUG
    printf("PS2: Device on port %d returned %d ID bytes: 0x%02X 0x%02X\n", port, id_count, id_bytes[0], id_bytes[1]);
    #endif
    #endif
    
    /*
		determine the DEVICE
	*/
    ps2_device_type_t device_type = PS2_DEVICE_NONE;
    
    if (id_count == 0) {
		/*
			OLD PS2 keyboard
		*/
        device_type = PS2_DEVICE_KEYBOARD_OLD;
        #ifdef DEBUG
        #ifdef DEBUG
        printf("PS2: Detected old AT keyboard on port %d\n", port);
        #endif
        #endif
    } else if (id_count == 2) {
        if (id_bytes[0] == 0xAB && id_bytes[1] == 0x83) {
			/*
				More modern Keyboard
			*/
            device_type = PS2_DEVICE_KEYBOARD_MF2;
            #ifdef DEBUG
            #ifdef DEBUG
            printf("PS2: Detected MF2 keyboard on port %d\n", port);
            #endif
            #endif
        } else if (id_bytes[0] == 0xAB && id_bytes[1] == 0x41) {
			/*
				Translation Keyboard but modern
			*/
            device_type = PS2_DEVICE_KEYBOARD_MF2;
            #ifdef DEBUG
            #ifdef DEBUG
            printf("PS2: Detected MF2 keyboard (translation) on port %d\n", port);
            #endif
            #endif
        } else if (id_bytes[0] == 0x00) {
			/*
				Standard Mouse
			*/
            device_type = PS2_DEVICE_MOUSE_STANDARD;
            #ifdef DEBUG
            #ifdef DEBUG
            printf("PS2: Detected standard mouse on port %d\n", port);
            #endif
            #endif
        } else if (id_bytes[0] == 0x03) {
			/*
				Modern mouse but still standard with wheel
			*/
            device_type = PS2_DEVICE_MOUSE_WHEEL;
            #ifdef DEBUG
            #ifdef DEBUG
            printf("PS2: Detected mouse with scroll wheel on port %d\n", port);
            #endif
            #endif
        } else if (id_bytes[0] == 0x04) {
			/*
				5 buttoned Mouse
			*/
            device_type = PS2_DEVICE_MOUSE_5BUTTON;
            #ifdef DEBUG
            #ifdef DEBUG
            printf("PS2: Detected 5-button mouse on port %d\n", port);
            #endif
            #endif
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


