#ifndef PS2_H
#define PS2_H
/*
	Header files
	Some legacy
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "keyps2.h"
#include "ps2mouse.h"
#include "../../../utilities/utility.h"
/*
	Pretty standard control ports
	data = 60
	status/command = 64
*/
#define PS2_DATA_PORT           0x60
#define PS2_STATUS_PORT         0x64
#define PS2_COMMAND_PORT        0x64
/*
	Controller commands
*/
#define PS2_CMD_READ_CONFIG     0x20
#define PS2_CMD_WRITE_CONFIG    0x60
#define PS2_CMD_DISABLE_PORT2   0xA7
#define PS2_CMD_ENABLE_PORT2    0xA8
#define PS2_CMD_TEST_PORT2      0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1      0xAB
#define PS2_CMD_DISABLE_PORT1   0xAD
#define PS2_CMD_ENABLE_PORT1    0xAE
#define PS2_CMD_WRITE_PORT2     0xD4
/*
	PS2 register 
*/
#define PS2_STATUS_OUTPUT_FULL  0x01
#define PS2_STATUS_INPUT_FULL   0x02
#define PS2_STATUS_SYSTEM_FLAG  0x04
#define PS2_STATUS_COMMAND_DATA 0x08
#define PS2_STATUS_TIMEOUT      0x40
#define PS2_STATUS_PARITY_ERROR 0x80
/*
	Some config
*/
#define PS2_CONFIG_PORT1_INT    0x01
#define PS2_CONFIG_PORT2_INT    0x02
#define PS2_CONFIG_SYSTEM_FLAG  0x04
#define PS2_CONFIG_PORT1_CLOCK  0x10
#define PS2_CONFIG_PORT2_CLOCK  0x20
#define PS2_CONFIG_PORT1_TRANS  0x40
/*
	Device command
*/
#define PS2_DEV_RESET           0xFF
#define PS2_DEV_DISABLE_SCAN    0xF5
#define PS2_DEV_ENABLE_SCAN     0xF4
#define PS2_DEV_IDENTIFY        0xF2
#define PS2_DEV_SET_SCAN_SET    0xF0
#define PS2_DEV_ECHO            0xEE
/*
	Constants for response
*/
#define PS2_RESP_ACK            0xFA
#define PS2_RESP_RESEND         0xFE
#define PS2_RESP_ERROR          0xFC
#define PS2_RESP_SELF_TEST_OK   0xAA
#define PS2_RESP_ECHO           0xEE
/*
	Some PS2 mouse
*/
#define PS2_MOUSE_SET_DEFAULTS  0xF6
#define PS2_MOUSE_SET_SAMPLE    0xF3
#define PS2_MOUSE_GET_ID        0xF2
#define PS2_MOUSE_ENABLE_DATA   0xF4
#define PS2_MOUSE_DISABLE_DATA  0xF5
#define PS2_MOUSE_SET_STREAM    0xEA
#define PS2_MOUSE_STATUS_REQ    0xE9
#define PS2_MOUSE_SET_RESOLUTION 0xE8
/*
	Structures
*/
typedef enum {
    PS2_DEVICE_NONE = 0,
    PS2_DEVICE_KEYBOARD_OLD, /* Old PS2 keyboard */
    PS2_DEVICE_KEYBOARD_MF2, /* More stanrd PS2 keyboard */
    PS2_DEVICE_MOUSE_STANDARD, /* Standard PS2 mouse */
    PS2_DEVICE_MOUSE_WHEEL, /* Mouse with wheel */
    PS2_DEVICE_MOUSE_5BUTTON /* mouse with more then 3 buttons */
} ps2_device_type_t;
/*
	Controller
*/
typedef struct {
    bool dual_channel;
    bool port1_exists;
    bool port2_exists;
    ps2_device_type_t port1_device;
    ps2_device_type_t port2_device;
    uint8_t config_byte;
    bool initialized;
} ps2_controller_t;
/*
	Globals
*/
extern ps2_controller_t ps2_controller;
/*
	Prototypes
*/
bool ps2_wait_input(void);
bool ps2_wait_output(void);
void ps2_write_command(uint8_t command);
void ps2_write_data(uint8_t data);
uint8_t ps2_read_data(void);
bool ps2_read_data_timeout(uint8_t *data, uint32_t timeout_ms);
bool ps2_send_device_command(uint8_t port, uint8_t command);
bool ps2_send_device_command_with_data(uint8_t port, uint8_t command, uint8_t data);
ps2_device_type_t ps2_identify_device(uint8_t port);
bool ps2_is_initialized(void);
#endif