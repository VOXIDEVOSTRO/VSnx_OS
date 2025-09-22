#ifndef VBOX_H
#define VBOX_H
/*
	Header files
*/
#include "../../../hookorder.h"
/*
	Constants
*/
#define VENDOR_VIRTUALBOX 0x80EE
#define PCI_CLASS_DISPLAY 0x03
/*
	prototypes
*/
int vboxsvga_spawn_driver(void);
#endif