/*
	header files
*/
#include "../hal/interrupts/interrupts.h"
#include "thread.h"
/*
	PREEMPTIVE schedular
	round robin
*/
void scheduler_tick(interrupt_frame_t* frame) {
	/*
		Check if queues and threads exists
	*/
    if (!current_thread || !ready_queue) return;
	/*
		find the next thread
	*/
    if (current_thread) {
        current_thread->context = *frame;
        current_thread->state = THREAD_READY;
        
        thread_t* last = ready_queue;
        while (last->next) last = last->next;
        last->next = current_thread;
        current_thread->next = NULL;
    }
    /*
		Runn it
	*/
    current_thread = ready_queue;
    ready_queue = ready_queue->next;
    current_thread->next = NULL;
    current_thread->state = THREAD_RUNNING;
    /*
		Handle the privilege level
	*/
    if (current_thread->privilege == THREAD_RING3) {
		/*
			ring 3
		*/
        // Update TSS with thread's kernel stack
        extern tss_t tss;
        tss.rsp0 = current_thread->kernel_stack + KERNEL_STACK_SIZE - 8;
        
        // Set up Ring 3 transition frame
        frame->rax = current_thread->context.rax;
        frame->rbx = current_thread->context.rbx;
        frame->rcx = current_thread->context.rcx;
        frame->rdx = current_thread->context.rdx;
        frame->rsi = current_thread->context.rsi;
        frame->rdi = current_thread->context.rdi;
        frame->rbp = current_thread->context.rbp;
        frame->r8 = current_thread->context.r8;
        frame->r9 = current_thread->context.r9;
        frame->r10 = current_thread->context.r10;
        frame->r11 = current_thread->context.r11;
        frame->r12 = current_thread->context.r12;
        frame->r13 = current_thread->context.r13;
        frame->r14 = current_thread->context.r14;
        frame->r15 = current_thread->context.r15;
        /*
			Setup stack frame 
		*/
        frame->rip = current_thread->context.rip;
        frame->cs = USER_CODE_SELECTOR;
        frame->rflags = current_thread->context.rflags | 0x200;
        frame->rsp = current_thread->context.rsp;
        frame->ss = USER_DATA_SELECTOR;
    } else {
		/*
			Ring 0
		*/
        *frame = current_thread->context;
    }
}
