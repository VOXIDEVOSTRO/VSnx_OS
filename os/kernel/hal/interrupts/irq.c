/*
	Header files
*/
#include "interrupts.h"
#include "../../threading/thread.h"
#include "../io/ps2/ps2.h"
#include "../../systemclock/clock.h"
/*
    IRQ Handler - Hardware interrupt handling
*/
void irq_handler(interrupt_frame_t* frame) {
    /*
		Also make sure its a valid frame
	*/
    if (!frame/*NULL*/) return;
    /*
		Validate the IRQ
	*/
    uint32_t irq_num = frame->int_no - 32; // calculate the irq number by: the IRQx , x = interrupt number - 32, thats maths baby
    if (irq_num > 15) return;
    
    /*
		EOI please
	*/
    if (irq_num >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
    /*
		case switch for handling the IRQs
		because easier and more optimized then an elseif
		(Learned from my older VOSTROX)
	*/
    switch (irq_num) {
        case 0:
			/*
				Schedular duh...
			*/
			scheduler_tick(frame);
			system_clock_tick(/*UPDATE the clock*/);
            break;
        case 1:
            ps2_keyboard_interrupt_handler(frame);
            break;
        default:
            break;
    }
}