#ifndef PS2DECT_H
#define PS2DECT_H
/*
	Header files
*/
#include <stdbool.h>
#include <stdint.h>
#include "../../../utilities/utility.h"
#include "../../../hal/io/io_ports.h"
#include "../../../elf/process.h"
#include "../../../threading/thread.h"
/*
	PS2 Port definitions
*/
#define PS2_DATA_PORT           0x60
#define PS2_STATUS_PORT         0x64
#define PS2_COMMAND_PORT        0x64
/*
	PS2 Commands
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
	PS2 Status bits
*/
#define PS2_STATUS_OUTPUT_FULL  0x01
#define PS2_STATUS_INPUT_FULL   0x02
/*
	PS2 Responses
*/
#define PS2_RESP_ACK            0xFA
#define PS2_RESP_RESEND         0xFE
#define PS2_RESP_ERROR          0xFC
#define PS2_RESP_SELF_TEST_OK   0xAA
/*
	Device Commands
*/
#define PS2_DEV_RESET           0xFF
#define PS2_DEV_IDENTIFY        0xF2
/*
	PS2 Controller and Device Types
*/
typedef enum {
    PS2_DEVICE_NONE = 0,
    PS2_DEVICE_KEYBOARD_OLD,
    PS2_DEVICE_KEYBOARD_MF2,
    PS2_DEVICE_MOUSE_STANDARD,
    PS2_DEVICE_MOUSE_WHEEL,
    PS2_DEVICE_MOUSE_5BUTTON
} ps2_device_type_t;
/*
	PS2 Controller Structure
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
bool ps2_controller_detect(void);
bool ps2_keyboard_detect(void);
bool ps2_mouse_detect(void);
void ps2_devices_init(void);
ps2_device_type_t ps2_identify_device(uint8_t port);
#endif
