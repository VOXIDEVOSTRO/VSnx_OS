#ifndef KBD_H
#define KBD_H
/*
	Head
*/
#include "../../hook.h"
#include "../../../hal/interrupts/interrupts.h"
/*
	Prototypes
	I guess for now these are fine...
*/
void kbd_init(void);
void kbd_shutdown(void);
void kbd_has_key(uint32_t *has_key_out);
void kbd_get_key(uint32_t *keycode_out); /*Getchar*/
void kbd_query_state(uint32_t keycode, uint32_t *pressed_out);
void kbd_set_leds(uint32_t led_mask);
void kbd_get_modifiers(uint32_t *mods_out); /*The modifers like alt, ctrl, shift and etc...*/
void kbd_translate(uint32_t keycode, uint32_t *unicode_out);
void kbd_interrupt_handler(uint64_t* context);
#endif