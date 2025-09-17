/*
	Headerz
*/
#include "clock.h"
/*
	Some global declaring
*/
volatile uint32_t tick_count = 0;
struct system_time current_time;
/*
	MAIN thing
	will be put in the IRQ handler
*/
void system_clock_tick() {
    tick_count++;
    /*
		Update
	*/
    if (tick_count % 1000 == 0) {
		/*
			Seconds
		*/
        current_time.seconds++;
		/*
			Minutes
		*/
        if (current_time.seconds >= 60) {
            current_time.seconds = 0;
            current_time.minutes++;
            /*
				Hours
			*/
            if (current_time.minutes >= 60) {
                current_time.minutes = 0;
                current_time.hours++;
                /*
					days
					Idk who keeps thier computers on for
					DAYS
				*/
                if (current_time.hours >= 24) {
                    current_time.hours = 0;
                    current_time.days++;
                }
            }
        }
    }
	
	/*
		Milliseconds
	*/
    current_time.milliseconds = tick_count % 1000;
}
/*
	main init
*/
void init_system_clock() {
	/*
		Init the VARS
	*/
    current_time.milliseconds = 0;
    current_time.seconds = 0;
    current_time.minutes = 0;
    current_time.hours = 0;
    current_time.days = 0;
}
/*
	Simple time giver
*/
void get_system_time(struct system_time* time) {
    time->milliseconds = current_time.milliseconds;
    time->seconds = current_time.seconds;
    time->minutes = current_time.minutes;
    time->hours = current_time.hours;
    time->days = current_time.days;
}

/*
	Also some API's
*/

uint64_t time_now_ms(void) {
    struct system_time t1, t2;
    do {
        get_system_time(&t1);
        get_system_time(&t2);
    } while (t1.milliseconds != t2.milliseconds ||
             t1.seconds      != t2.seconds      ||
             t1.minutes      != t2.minutes      ||
             t1.hours        != t2.hours        ||
             t1.days         != t2.days);

    uint64_t ms = 0;
    ms += (uint64_t)t1.days    * 24ULL * 60ULL * 60ULL * 1000ULL;
    ms += (uint64_t)t1.hours   * 60ULL * 60ULL * 1000ULL;
    ms += (uint64_t)t1.minutes * 60ULL * 1000ULL;
    ms += (uint64_t)t1.seconds * 1000ULL;
    ms += (uint64_t)t1.milliseconds;
    return ms;
}

uint32_t uptime_seconds(void) {
    return (uint32_t)(time_now_ms() / 1000ULL);
}

uint32_t uptime_minutes(void) {
    return (uint32_t)(time_now_ms() / 60000ULL);
}

int time_after(uint64_t a, uint64_t b) {
    return (int64_t)(b - a) < 0;
}

int time_before(uint64_t a, uint64_t b) {
    return (int64_t)(a - b) < 0;
}
