#ifndef MOUSE_H
#define MOUSE_H
/*
	headerz
*/
#include "ps2.h"
/*
	Strctures
*/
typedef struct {
    int32_t x;
    int32_t y;
    uint8_t buttons;
    int8_t wheel;
    uint8_t packet[4];
    uint8_t packet_index;
    bool packet_ready;
} mouse_state_t;
/*
	Globes (globals?? or is it???? *vsauce music starts playing*)
*/
extern mouse_state_t mouse_state;
/*
	prototypes
*/
void ps2_mouse_init(uint8_t port);
bool ps2_has_mouse(void);
#endif