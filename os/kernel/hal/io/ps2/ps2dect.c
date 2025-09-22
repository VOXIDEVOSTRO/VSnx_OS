/*
	Header files
*/
#include "ps2dect.h"
/*
	PS2 Controller Global
*/
ps2_controller_t ps2_controller = {0};
/*
	Helper functions for PS2 communication
*/
static bool ps2_wait_output(void) {
    for (int i = 0; i < 100000; i++) {
        if (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            return true;
        }
    }
    return false;
}

static bool ps2_wait_input(void) {
    for (int i = 0; i < 100000; i++) {
        if (!(inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL)) {
            return true;
        }
    }
    return false;
}

static void ps2_write_command(uint8_t command) {
    ps2_wait_input();
    outb(PS2_COMMAND_PORT, command);
}

static void ps2_write_data(uint8_t data) {
    ps2_wait_input();
    outb(PS2_DATA_PORT, data);
}

static uint8_t ps2_read_data(void) {
    ps2_wait_output();
    return inb(PS2_DATA_PORT);
}
/*
	Simple sender
*/
static bool ps2_send_device_command(uint8_t port, uint8_t command) {
    if (port == 2) {
        ps2_write_command(PS2_CMD_WRITE_PORT2);
    }
    ps2_write_data(command);

    if (!ps2_wait_output()) return false;
    uint8_t response = ps2_read_data();

    return (response == PS2_RESP_ACK);
}
/*
	Controller detection because,
	we need the controller first or verify
*/
bool ps2_controller_detect(void) {
	/*
		using the test may give a idea
	*/
    ps2_write_command(PS2_CMD_TEST_CONTROLLER);
    if (!ps2_wait_output()) {
        #ifdef DEBUG
        printf("PS2 Controller test timeout\n");
        #endif
        return false;
    }
	/*
		Read response
	*/
    uint8_t response = ps2_read_data();
    if (response == PS2_RESP_SELF_TEST_OK || response == 0x55) {
        #ifdef DEBUG
        printf("PS2 Controller detected and working (response: 0x%x)\n", response);
        #endif
        ps2_controller.initialized = true;
        return true;
    } else {
        #ifdef DEBUG
        printf("PS2 Controller test failed (response: 0x%x)\n", response);
        #endif
        return false;
    }
}
/*
	simple device identification
*/
ps2_device_type_t ps2_identify_device(uint8_t port) {
    if (!ps2_send_device_command(port, PS2_DEV_IDENTIFY)) {
        #ifdef DEBUG
        printf("PS2 Port %d: IDENTIFY command failed\n", port);
        #endif
        return PS2_DEVICE_NONE;
    }
	/*
		Wait and read the first
	*/
    if (!ps2_wait_output()) {
        #ifdef DEBUG
        printf("PS2 Port %d: No response to IDENTIFY\n", port);
        #endif
        return PS2_DEVICE_NONE;
    }
    uint8_t id1 = ps2_read_data();
	/*
		Then also for the second
	*/
    if (ps2_wait_output()) {
        uint8_t id2 = ps2_read_data();
        #ifdef DEBUG
        printf("PS2 Port %d: Device ID 0x%x 0x%x\n", port, id1, id2);
        #endif
		/*
			Keyboard types
		*/
        if (id1 == 0xAB && (id2 >= 0x41 && id2 <= 0x47)) {
            return PS2_DEVICE_KEYBOARD_MF2;
        }
		/*
			Mouse types
		*/
        if (id1 == 0x00) {
            return PS2_DEVICE_MOUSE_STANDARD;
        }
        if (id1 == 0x03) {
            return PS2_DEVICE_MOUSE_WHEEL;
        }
        if (id1 == 0x04) {
            return PS2_DEVICE_MOUSE_5BUTTON;
        }
    } else {
        #ifdef DEBUG
        printf("PS2 Port %d: Single byte response 0x%x\n", port, id1);
        #endif
		/*
			Mouse types (single byte)
		*/
        if (id1 == 0x00) {
            return PS2_DEVICE_MOUSE_STANDARD;
        }
        if (id1 == 0x03) {
            return PS2_DEVICE_MOUSE_WHEEL;
        }
        if (id1 == 0x04) {
            return PS2_DEVICE_MOUSE_5BUTTON;
        }
		/*
			If single byte response, assume keyboard
		*/
        if (id1 != 0xFF && id1 != 0xFE && id1 != 0xFC) {
            return PS2_DEVICE_KEYBOARD_OLD;
        }
    }

    return PS2_DEVICE_NONE;
}
/*
	Keyboard detection
*/
bool ps2_keyboard_detect(void) {
	/*
		IDENTIFY YOURSELF!
	*/
    ps2_device_type_t dev_type = ps2_identify_device(1);
    if (dev_type == PS2_DEVICE_KEYBOARD_OLD || dev_type == PS2_DEVICE_KEYBOARD_MF2) {
        #ifdef DEBUG
        printf("PS2 Keyboard detected on port 1\n");
        #endif
        ps2_controller.port1_device = dev_type;
        ps2_controller.port1_exists = true;
        return true;
    } else {
		/*
			Ah crap...
		*/
        #ifdef DEBUG
        printf("No PS2 Keyboard detected on port 1\n");
        #endif
        return false;
    }
}
/*
	Mouse detection
*/
bool ps2_mouse_detect(void) {
	/*
		Any port 2
	*/
    ps2_write_command(PS2_CMD_ENABLE_PORT2);
	/*
		Test the port
	*/
    ps2_write_command(PS2_CMD_TEST_PORT2);
    if (ps2_wait_output()) {
        uint8_t response = ps2_read_data();
		/*
			DONE
		*/
        if (response == 0x00) {
            ps2_controller.dual_channel = true;

			/*
				Identify...
			*/
            ps2_device_type_t dev_type = ps2_identify_device(2);
            if (dev_type >= PS2_DEVICE_MOUSE_STANDARD) {
                #ifdef DEBUG
                printf("PS2 Mouse detected on port 2\n");
                #endif
                ps2_controller.port2_exists = true;
                ps2_controller.port2_device = dev_type;
                return true;
            }
        }
    }

    #ifdef DEBUG
    printf("No PS2 Mouse detected on port 2\n");
    #endif
    return false;
}
/*
	Main PS2 devices initialization
*/
void ps2_devices_init(void) {
    #ifdef DEBUG
    printf("Initializing PS2 devices...\n");
    #endif
	/*
		Any controller?
	*/
    if (!ps2_controller_detect()) {
        #ifdef DEBUG
        printf("No PS2 controller found, skipping\n");
        #endif
        return;
    }
	/*
		Keyboard
	*/
    ps2_keyboard_detect();
	/*
		Mosue
	*/
    ps2_mouse_detect();
	/*
		Detect controller based devices
	*/
    uint8_t config = 0;

    if (ps2_controller.port1_exists) {
		/*
			enable interrupts
		*/
        config |= 0x01;
        ps2_write_command(PS2_CMD_ENABLE_PORT1);
    }

    if (ps2_controller.port2_exists) {
        config |= 0x02;
		/*
			Already done?
		*/
    }

	/*
		write up the config we set
	*/
    ps2_write_command(PS2_CMD_WRITE_CONFIG);
    ps2_write_data(config);
    ps2_controller.config_byte = config;	
	/*
		THEN load the drivers
	*/
    if (ps2_controller.port1_exists) {
        #ifdef DEBUG
        printf("Loading PS2 keyboard driver...\n");
        #endif
		/*
			Init the driver
			or load it
		*/
        process_t* ps2_keyboard_proc = spawn_process("/MODULES/SYS/DRIVERS/HID/PSTWO/KEYBRD/KEYPSTWO.ELF"/*as a mirror path*/,
													/*
														Its a driver so ring0
													*/
                                                     THREAD_RING0,
													/*
														Doesnt need alot of stuff...
													*/
                                                     THREAD_PRIORITY_BACKGROUND);

        if (ps2_keyboard_proc != NULL) {
            #ifdef DEBUG
            printf("PS2 keyboard driver process spawned (PID: %d)\n", ps2_keyboard_proc->pid);
            #endif
			/*
				VSnx rookie mistake: never executing the process...
				fixed up...
			*/
            if (execute_process(ps2_keyboard_proc) == 0) {
                #ifdef DEBUG
                printf("PS2 keyboard driver process executed successfully\n");
                #endif
            } else {
                #ifdef DEBUG
                printf("Failed to execute PS2 keyboard driver process\n");
                #endif
            }
        } else {
            #ifdef DEBUG
            printf("Failed to spawn PS2 keyboard driver process\n");
            #endif
        }
    }
	/*
		FOR NOW we dot have mouse
	*/
	#ifdef MOUSE
    if (ps2_controller.port2_exists) {
        #ifdef DEBUG
        printf("Loading PS2 mouse driver...\n");
        #endif

        process_t* ps2_mouse_proc = spawn_process("drivers/hid/ps2/mouse/mouse.bin",
                                                  THREAD_RING0,
                                                  THREAD_PRIORITY_BACKGROUND);

        if (ps2_mouse_proc != NULL) {
            #ifdef DEBUG
            printf("PS2 mouse driver process spawned (PID: %d)\n", ps2_mouse_proc->pid);
            #endif

			/*
				Execute...
			*/
            if (execute_process(ps2_mouse_proc) == 0) {
                #ifdef DEBUG
                printf("PS2 mouse driver process executed successfully\n");
                #endif
            } else {
                #ifdef DEBUG
                printf("Failed to execute PS2 mouse driver process\n");
                #endif
            }
        } else {
            #ifdef DEBUG
            printf("Failed to spawn PS2 mouse driver process\n");
            #endif
        }
    }
	#endif
	/*
		Dump as well
	*/
    #ifdef DEBUG
    printf("PS2 device initialization complete\n");
    #endif
    #ifdef DEBUG
    printf("Port 1: %s\n", ps2_controller.port1_exists ? "Keyboard" : "None");
    #endif
    #ifdef DEBUG
    printf("Port 2: %s\n", ps2_controller.port2_exists ? "Mouse" : "None");
    #endif
}
