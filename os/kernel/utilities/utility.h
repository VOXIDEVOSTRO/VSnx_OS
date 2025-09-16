#ifndef UTIL_H
#define UTIL_H
/*
	HEADERS
*/
#include "../hal/serial_com/serial.h" // for both serial and VGA text
#include "../hal/vga/vga_text/vga_text.h"
#include "strings/strings.h"

/*


	DEBUG
	because here this is a common file
	And useful for debugging and also optimize


*/


//#define DEBUG 				/*UNCOMMENT THIS TO SEE EXTRA OUTPUT!*/
//#define FAT_DEBUG 			/*UNCOMMENT THIS TO SEE EXTRA OUTPUT! for gristle driver*/


/*
	STRUCTURES
*/
typedef enum {
    OUTPUT_SERIAL = 1,
    OUTPUT_VGA = 2,
    OUTPUT_BOTH = 3
} output_target_t;
/*
	Glboals
*/
extern output_target_t current_output;
/*
	prototypes
*/
/*
	printf.c
*/
void putchar_output(char c);
void print_string(const char* str);
void print_number(uint64_t num, int base, int uppercase);
void print_signed(int64_t num, int base);
/*UNUSED*/ //void print_hex(uint64_t num, int uppercase, int prefix);
int printf(const char* format, ...);
void set_printf_output(int target) ;
#endif