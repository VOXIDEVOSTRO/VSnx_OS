#ifndef CLOCK_H
#define CLOCK_H
/*
	Header filez
*/
#include <stdint.h>
/*
	Systemtime structure
*/
struct system_time {
    uint32_t milliseconds;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint32_t days;
};
extern struct system_time current_time;
/*
	some globals
*/
extern volatile uint32_t tick_count;
/*
	Prototypes
*/
#ifndef UCLOCK
void init_system_clock(void);
void system_clock_tick(void);
void get_system_time(struct system_time* time);
#endif
#endif