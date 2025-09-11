#ifndef SERIAL_H
#define SERIAL_H
/*
	Standards
*/
#include <stdint.h>
#include "../io/io_ports.h"
/*
	MACROS
*/
/*
	Try both COM ports
	So that if one isn't responding, 
	the other will be. (probably)
*/
#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8
/*
	Serial port registers
*/
#define SERIAL_DATA_REG(port)     (port) // 0
#define SERIAL_INT_EN_REG(port)   (port + 1)
#define SERIAL_FIFO_REG(port)     (port + 2)
#define SERIAL_LINE_REG(port)     (port + 3)
#define SERIAL_MODEM_REG(port)    (port + 4)
#define SERIAL_STATUS_REG(port)   (port + 5)
/*
	LCB (Line control bits)
*/
#define SERIAL_8N1 0x03
#define SERIAL_DLAB 0x80
/*
	status bits
*/
#define SERIAL_TX_READY 0x20
#define SERIAL_RX_READY 0x01
/*
	Prototypes
*/
void serial_init(uint16_t port);
void serial_putchar(uint16_t port, char c);
void serial_print(uint16_t port, const char* str);
#endif