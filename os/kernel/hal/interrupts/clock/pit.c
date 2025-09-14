/*
	Headerz
	or header files
*/
#include "pit.h"
/*
    Set PIT frequency in Hz
*/
void set_freq(uint32_t frequency) {
    if (frequency == 0) { // NULL check
        #ifdef DEBUG
        printf("PIT: Invalid frequency 0\n");
        #endif
        return;
    }
    
	/*
		calc divisor
		div = base / freq
	*/
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

	/*
		Clamp here:
		because we can change via the set_div
	*/
    if (divisor > 65535) {
        divisor = 65535;
		/*
			That would be slow asf
		*/
        #ifdef DEBUG
        printf("PIT: Frequency too low, clamped to %u Hz\n", PIT_BASE_FREQUENCY / 65535);
        #endif
    }
    if (divisor < 1) {
        divisor = 1;
		/*
			To avoid overwhelming
		*/
        #ifdef DEBUG
        printf("PIT: Frequency too high, clamped to %u Hz\n", PIT_BASE_FREQUENCY);
        #endif
    }
    
    #ifdef DEBUG
    printf("PIT: Setting frequency to %u Hz (divisor=%u)\n", PIT_BASE_FREQUENCY / divisor, divisor);
    #endif
	/*
		main setting
	*/
    outb(PIT_COMMAND_PORT, 0x36);  // Channel 0, lobyte/hibyte, rate generator
    outb(PIT_DATA_PORT, divisor & 0xFF);        // Low byte
    outb(PIT_DATA_PORT, (divisor >> 8) & 0xFF); // High byte
}

/*
    Set PIT using raw divisor value

*/
void set_div(uint16_t divisor) {
    if (divisor == 0) {
        #ifdef DEBUG
        printf("PIT: Invalid divisor 0\n");
        #endif
        return;
    }
    
	/*
		Simple divsior set
	*/
    uint32_t frequency = PIT_BASE_FREQUENCY / divisor;
    #ifdef DEBUG
    printf("PIT: Setting divisor %u (frequency=%u Hz)\n", divisor, frequency);
    #endif
    
    /*
		Set divisor:
		Channel 0, lobyte/hibyte, rate generator
    	Low byte
    	High byte.
		Thats it...
	*/
    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_DATA_PORT, divisor & 0xFF);
    outb(PIT_DATA_PORT, (divisor >> 8) & 0xFF);
}
/*
	PIT dumper
	we will get for system clock
	and maybe some debug
*/
#ifdef DUMP_PIT
void pit_dump(void) {
    printf("=== PIT SETTINGS ===\n");
    
    outb(PIT_COMMAND_PORT, 0xE2);
    
	/*
		read status
	*/
    uint8_t status = inb(PIT_DATA_PORT);
    
    /*
		read the count first
	*/
    uint16_t current_count = 0;
    if (!(status & 0x40)) {  /*count*/
        uint8_t low = inb(PIT_DATA_PORT);
        uint8_t high = inb(PIT_DATA_PORT);
        current_count = (high << 8) | low;
    }
    
    /*
		decode duh...
	*/
    uint8_t bcd = (status & 0x01);
    uint8_t mode = (status >> 1) & 0x07;
    uint8_t access = (status >> 4) & 0x03;
    uint8_t null_count = (status >> 6) & 0x01;
    uint8_t output_pin = (status >> 7) & 0x01;
    
	/*
		DUMP UP
	*/
    printf("Status: 0x%02X\n", status);
    printf("BCD Mode: %s\n", bcd ? "BCD" : "Binary");
    printf("Operating Mode: %u\n", mode);
    printf("Access Mode: %s\n", 
           access == 1 ? "lobyte only" :
           access == 2 ? "hibyte only" :
           access == 3 ? "lobyte/hibyte" : "latch count");
    printf("NULL Count: %s\n", null_count ? "Yes" : "No");
    printf("Output Pin: %s\n", output_pin ? "High" : "Low");
    
    if (!null_count && current_count > 0) {
        uint32_t frequency = PIT_BASE_FREQUENCY / current_count;
        printf("Current Divisor: %u\n", current_count);
        printf("Current Frequency: %u Hz\n", frequency);
        printf("Timer Interval: %u.%03u ms\n", 
               1000 / frequency, (1000 % frequency) * 1000 / frequency);
    } else {
        printf("Current Count: Not available\n");
    }
    
    /*Np idea but a footer is always nice marker for serial*/printf("==================\n");
}
#endif