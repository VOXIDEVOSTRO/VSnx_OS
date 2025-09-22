/*
	Headers
*/
#include "keybrd.h"
/*
	Main hook wrappers
*/
void kbd_init(void) {
    struct hook_data data = {0};
    HOOK_CALL("kbd_init", &data);
}
void kbd_shutdown(void) {
    struct hook_data data = {0};
    HOOK_CALL("kbd_shutdown", &data);
}

void kbd_has_key(uint32_t *has_key_out) {
    struct hook_data data = {0};
    HOOK_CALL("kbd_has_key", &data);
    *has_key_out = data.args[0];
}
void kbd_get_key(uint32_t *keycode_out) {
    struct hook_data data = {0};
    HOOK_CALL("kbd_get_key", &data);
    *keycode_out = data.args[0];
}
void kbd_query_state(uint32_t keycode, uint32_t *pressed_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)keycode;
    HOOK_CALL("kbd_query_state", &data);
    *pressed_out = data.args[0];
}
void kbd_set_leds(uint32_t led_mask) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)led_mask;
    HOOK_CALL("kbd_set_leds", &data);
}
void kbd_get_modifiers(uint32_t *mods_out) {
    struct hook_data data = {0};
    HOOK_CALL("kbd_get_modifiers", &data);
    *mods_out = data.args[0];
}
void kbd_translate(uint32_t keycode, uint32_t *unicode_out) {
    struct hook_data data = {0};
    data.args[0] = (uint64_t)keycode;
    HOOK_CALL("kbd_translate", &data);
    *unicode_out = data.args[0];
}

void kbd_interrupt_handler(uint64_t* context) {
    struct hook_data data = {0};
    data.args[0] = context; 
    HOOK_CALL("kbd_interrupt_handler", &data);
}