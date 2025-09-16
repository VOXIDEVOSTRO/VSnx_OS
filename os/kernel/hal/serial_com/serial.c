/*
	HEADER FILES
	all included in one
*/
#include "serial.h"
/*
	Init the serial
*/
void serial_init(uint16_t port) {
    outb(SERIAL_INT_EN_REG(port), 0x00);    	// Disable interrupts
    outb(SERIAL_LINE_REG(port), SERIAL_DLAB); 	// Enable DLAB
    outb(SERIAL_DATA_REG(port), 0x03);      	// set divisor low
    outb(SERIAL_INT_EN_REG(port), 0x00);    	// set divisor high
    outb(SERIAL_LINE_REG(port), SERIAL_8N1); 	// 8N1, disable DLAB
    outb(SERIAL_FIFO_REG(port), 0xC7);      	// Enable FIFO, clear, 14-byte threshold
    outb(SERIAL_MODEM_REG(port), 0x0B);     	// irqs enabled, RTS/DSR set
}
/*
	Print functions [print to serial]
*/
void serial_putchar(uint16_t port, char c) { // PUTS or putchar instead
    while (!(inb(SERIAL_STATUS_REG(port)) & SERIAL_TX_READY));
    outb(SERIAL_DATA_REG(port), c);
}
void serial_print(uint16_t port, const char* str) { // PRINTS
    while (*str) {
        serial_putchar(port, *str++);
    }
}