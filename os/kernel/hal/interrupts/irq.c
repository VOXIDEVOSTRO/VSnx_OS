/*
	Header files
*/
#include "interrupts.h"
#include "../../threading/thread.h"
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
    uint32_t irq_num = frame->int_no - 32; // calculate the irq number by: the IRQx , x = interrupt number - 32
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
	*/
    switch (irq_num) {
        case 0:
			/*
				Schedular duh...
			*/
			scheduler_tick(frame);
            break;
        case 1:  // Keyboard - just return for now
            break;
        default:
            break;
    }
}