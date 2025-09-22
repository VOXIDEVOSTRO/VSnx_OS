#ifndef GPU_H
#define	GPU_H
/*
	Hook includes
*/
#include "../../hookorder.h"
/*
	VBOX
*/
#include "virtualbox/vbox.h"
/*
	VGA
*/
#include "vga/vga.h"
/*
	Prototypes
*/
int gpu_load_drivers(void);
#endif