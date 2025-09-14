#ifndef PIT_H
#define PIT_H
/*
	HEaderz
*/
#include <stdint.h>
#include "../../../utilities/utility.h"
//#define DEBUG /*UNCOMMENT THIS TO SEE EXTRA OUTPUT!*/

/*
	simple constants
*/
#define PIT_BASE_FREQUENCY  1193182
#define PIT_COMMAND_PORT    0x43
#define PIT_DATA_PORT       0x40
/*
	prototypes
*/
void pit_set_freq(uint32_t frequency);
void pit_set_div(uint16_t divisor);
void pit_dump_settings(void);
#endif