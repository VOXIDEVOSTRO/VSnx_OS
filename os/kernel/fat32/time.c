/*
    Stub implementations for Gristle FAT driver
	all are just stubs becuase we dont have the time yet (pun) tbh thats bad pun
*/
#include "time.h"

// Simple counter for fake time (idk but just keep this shi static)
static time_t fake_time_counter = 1000000000; // Start at year 2001

/*
    Get current time (stub implementation)
*/
time_t time(time_t* timer) {
    // Increment fake time counter
    fake_time_counter++;
    
    if (timer) {
        *timer = fake_time_counter;
    }
    
    return fake_time_counter;
}

/*
    Convert time_t to broken-down time (stub implementation)
*/
struct tm* gmtime(const time_t* timer) {
    static struct tm tm_buf;
    
    // Simple stub - return a fixed date/time
    tm_buf.tm_sec = 0;      // 0 seconds
    tm_buf.tm_min = 0;      // 0 minutes  
    tm_buf.tm_hour = 12;    // 12 noon
    tm_buf.tm_mday = 1;     // 1st day
    tm_buf.tm_mon = 0;      // January (0-based)
    tm_buf.tm_year = 124;   // 2024 (years since 1900)
    tm_buf.tm_wday = 1;     // Monday
    tm_buf.tm_yday = 0;     // 1st day of year
    tm_buf.tm_isdst = -1;   // DST info not available
    
    return &tm_buf;
}

/*
    Convert broken-down time to time_t (stub implementation)
*/
time_t mktime(struct tm* timeptr) {
    if (!timeptr) {
        return -1;
    }
    
    // Simple stub - just return current fake time
    return fake_time_counter;
}
