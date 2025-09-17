#ifndef TIME_H
#define TIME_H
/*
	Header files
*/
#include <stdint.h>
/*
    Time types
*/
typedef long time_t;
/*
	UNIX time?
*/
struct tm {
    int tm_sec;     // Seconds (0-60)
    int tm_min;     // Minutes (0-59)
    int tm_hour;    // Hours (0-23)
    int tm_mday;    // Day of month (1-31)
    int tm_mon;     // Month (0-11)
    int tm_year;    // Years since 1900
    int tm_wday;    // Day of week (0-6, Sunday = 0)
    int tm_yday;    // Day of year (0-365)
    int tm_isdst;   // Daylight saving time flag
};
/*
    Time function prototypes
*/
time_t time(time_t* timer);
struct tm* gmtime(const time_t* timer);
time_t mktime(struct tm* timeptr);

#endif
