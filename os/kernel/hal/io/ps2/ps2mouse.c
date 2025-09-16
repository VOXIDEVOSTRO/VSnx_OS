/*
	Header filez
*/
#include "ps2mouse.h"
/*
	Declared globals
*/
mouse_state_t mouse_state = {0};
/*
	Some functions
*/
void ps2_mouse_init(uint8_t port) {
    #ifdef DEBUG
    printf("PS2: Initializing mouse on port %d\n", port);
    #endif
    
    /*
		reset to avoid CHAOS
	*/
    if (ps2_send_device_command(port, PS2_DEV_RESET)) {
        uint8_t response;
        if (ps2_read_data_timeout(&response, 1000)) {
            if (response == PS2_RESP_SELF_TEST_OK) {
                #ifdef DEBUG
                printf("PS2: Mouse self-test passed\n");
                #endif
                
                // Read mouse ID
                uint8_t mouse_id;
                if (ps2_read_data_timeout(&mouse_id, 100)) {
                    #ifdef DEBUG
                    printf("PS2: Mouse ID: 0x%02X\n", mouse_id);
                    #endif
                }
            } else {
                #ifdef DEBUG
                printf("PS2: Mouse self-test failed: 0x%02X\n", response);
                #endif
            }
        }
    }
    
    // Set defaults
    ps2_send_device_command(port, PS2_MOUSE_SET_DEFAULTS);
    
    // Enable scroll wheel if supported
    if (ps2_controller.port2_device == PS2_DEVICE_MOUSE_WHEEL ||
        ps2_controller.port2_device == PS2_DEVICE_MOUSE_5BUTTON) {
        
        // Magic sequence to enable scroll wheel
        ps2_send_device_command_with_data(port, PS2_MOUSE_SET_SAMPLE, 200);
        ps2_send_device_command_with_data(port, PS2_MOUSE_SET_SAMPLE, 100);
        ps2_send_device_command_with_data(port, PS2_MOUSE_SET_SAMPLE, 80);
        
        // Get device ID again
        if (port == 2) {
            ps2_write_command(PS2_CMD_WRITE_PORT2);
        }
        ps2_write_data(PS2_MOUSE_GET_ID);
        
        uint8_t response, new_id;
        if (ps2_read_data_timeout(&response, 100) && response == PS2_RESP_ACK) {
            if (ps2_read_data_timeout(&new_id, 100)) {
                #ifdef DEBUG
                printf("PS2: Mouse ID after wheel enable: 0x%02X\n", new_id);
                #endif
            }
        }
    }
    
    // Set sample rate
    ps2_send_device_command_with_data(port, PS2_MOUSE_SET_SAMPLE, 100);
    
    // Enable data reporting
    ps2_send_device_command(port, PS2_MOUSE_ENABLE_DATA);
    
    #ifdef DEBUG
    printf("PS2: Mouse initialization complete\n");
    #endif
}
bool ps2_has_mouse(void) {
    return ps2_controller.port2_device >= PS2_DEVICE_MOUSE_STANDARD;
}
