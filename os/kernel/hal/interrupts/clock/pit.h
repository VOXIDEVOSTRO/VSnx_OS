#ifndef PIT_H
#define PIT_H
/*
	HEaderz
*/
#include <stdint.h>
#include "../../../utilities/utility.h"

/*
	simple constants
*/
#define PIT_BASE_FREQUENCY  1193182
#define PIT_COMMAND_PORT    0x43
#define PIT_DATA_PORT       0x40
/*
	prototypes
*/
void set_freq(uint32_t frequency);
void set_div(uint16_t divisor);
void pit_dump(void);
#endif