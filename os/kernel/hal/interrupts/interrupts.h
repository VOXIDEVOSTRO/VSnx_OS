#ifndef INTERR_H
#define INTERR_H
/*
	header files
*/
#include "gdt/gdt.h"
#include "idt/idt.h"
/*
	prototypes
*/
void irq_handler(interrupt_frame_t* frame);
void isr_handler(interrupt_frame_t* frame);
#endif