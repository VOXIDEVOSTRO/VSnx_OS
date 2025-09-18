/*
	HEADER FILES
*/
#include "utility.h"
/*
	DEFINE DEBUG:
	This is for extra ouput which uses the printf for debugging here.
*/
/*
	Declare globals HERE
*/
output_target_t current_output = OUTPUT_BOTH;
/*
    HELPER FUNCTIONS
	for the printf
*/
void putchar_output(char c) {
    if (current_output & OUTPUT_SERIAL) {
        serial_putchar(COM1_PORT, c);
    }
	/*
		we puting both em atually
	*/
    if (current_output & OUTPUT_VGA) {
        vga_putchar(c);
    }
}

void print_string(const char* str) {
	/*
		Simple
	*/
    while (*str) {
		/*Both VGA and SERIAL*/
        putchar_output(*str++);
    }
}
/*
	so number with a padding?
*/
void print_number_padded(uint64_t num, int base, int uppercase, int width, char pad) {
    char buffer[32];
	/*
		Prolly handle both upper and lower
	*/
    char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;
    
    if (num == 0) {
        buffer[i++] = '0';
    } else {
        while (num > 0) {
            buffer[i++] = digits[num % base];
            num /= base;
        }
    }
	/*
		PAD
	*/
    while (i < width) {
        buffer[i++] = pad;
    }
	/*
		Reverse
	*/
    while (--i >= 0) {
        putchar_output(buffer[i]);
    }
}
/*
	Meaning negative digits
*/
void print_signed(int64_t num, int base) {
    if (num < 0) {
        putchar_output('-');
        num = -num;
    }
    print_number_padded((uint64_t)num, base, 0, 0, ' ');
}

/*
    MAIN PRINTF FUNCTION
	Our own printf as easier
*/
int printf(const char* format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    int count = 0;
    
    while (*format) {
        if (*format == '%') {
            format++;
            
			/*
				Parse up the width
			*/
            int width = 0;
            char pad = ' ';
            if (*format == '0') {
                pad = '0';
                format++;
            }
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            /*
            	Parse long modifier, because its printf
			*/
            int is_long = 0;
            if (*format == 'l') {
                is_long = 1;
                format++;
            }
            
            switch (*format) {
                case 'c': {
                    char c = (char)__builtin_va_arg(args, int);
                    putchar_output(c);
                    count++;
                    break;
                }
                case 's': {
                    const char* str = __builtin_va_arg(args, const char*);
                    if (!str) str = "(null)";
                    print_string(str);
                    count += strlen(str);
                    break;
                }
                case 'd':
                case 'i': {
                    if (is_long) {
                        int64_t num = __builtin_va_arg(args, int64_t);
                        print_signed(num, 10);
                    } else {
                        int num = __builtin_va_arg(args, int);
                        print_signed(num, 10);
                    }
                    count++;
                    break;
                }
                case 'u': {
                    if (is_long) {
                        uint64_t num = __builtin_va_arg(args, uint64_t);
                        print_number_padded(num, 10, 0, width, pad);
                    } else {
                        unsigned int num = __builtin_va_arg(args, unsigned int);
                        print_number_padded(num, 10, 0, width, pad);
                    }
                    count++;
                    break;
                }
                case 'x': {
                    if (is_long) {
                        uint64_t num = __builtin_va_arg(args, uint64_t);
                        print_number_padded(num, 16, 0, width, pad);
                    } else {
                        unsigned int num = __builtin_va_arg(args, unsigned int);
                        print_number_padded(num, 16, 0, width, pad);
                    }
                    count++;
                    break;
                }
                case 'X': {
                    if (is_long) {
                        uint64_t num = __builtin_va_arg(args, uint64_t);
                        print_number_padded(num, 16, 1, width, pad);
                    } else {
                        unsigned int num = __builtin_va_arg(args, unsigned int);
                        print_number_padded(num, 16, 1, width, pad);
                    }
                    count++;
                    break;
                }
                case 'p': {
                    void* ptr = __builtin_va_arg(args, void*);
                    print_string("0x");
                    print_number_padded((uint64_t)ptr, 16, 0, 16, '0');
                    count++;
                    break;
                }
                case '%': {
                    putchar_output('%');
                    count++;
                    break;
                }
                default: {
                    putchar_output('%');
                    putchar_output(*format);
                    count += 2;
                    break;
                }
            }
        } else {
            putchar_output(*format);
            count++;
        }
        format++;
    }
    
    __builtin_va_end(args);
    return count;
}

/*
    UTILITY FUNCTIONS or
	unwanted ones...
*/
void set_printf_output(int target) {
    current_output = (output_target_t)target;
}
