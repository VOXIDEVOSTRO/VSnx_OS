/*
	Header files
*/
#include "interrupts.h" // duh..
/*
    ISR Handler - Exception handling
	This time we will print more detailed information about the exception.
	because it will be easier to debug
*/
/*
    MASSIVE DEBUG INFO
*/
void isr_handler(interrupt_frame_t* frame) {
	/*
		first make sure we dont loop
		disable the interrupts
	*/
    __asm__ volatile("cli");
    
    printf("\n");
    printf("=====================================\n");
    printf("    VSNX EXCEPTION DEBUG REPORT\n");
    printf("=====================================\n");
    
	/*
		same
		validate frame
	*/
    if (!frame) {
        printf("FATAL: NULL interrupt frame!\n");
        while(1) __asm__ volatile("hlt");
    }
    
    // Exception type analysis
    printf("EXCEPTION INFO:\n");
    printf("  Exception Number: %u (0x%x)\n", frame->int_no, frame->int_no);
    printf("  Error Code: 0x%lx\n", frame->err_code);
    
    switch (frame->int_no) {
        case 0:
			/*
				divide by zero
			*/
            printf("  Type: DIVISION BY ZERO\n");
            printf("  Cause: Division by zero or DIV overflow\n");
            break;
        case 6:
			/*
				invalid opocde
			*/
            printf("  Type: INVALID OPCODE\n");
            printf("  Cause: Undefined instruction or corrupted code\n");
            break;
        case 8:
			/*
				Double fault
			*/
            printf("  Type: DOUBLE FAULT\n");
            printf("  Cause: Exception during exception handling\n");
            break;
        case 13:
			/*
				General protection fault
			*/
            printf("  Type: GENERAL PROTECTION FAULT\n");
            printf("  Cause: Segment violation or privilege violation\n");
            break;
        case 14:
			/*
				Page fault
			*/
            printf("  Type: PAGE FAULT\n");
            printf("  Cause: Invalid memory access\n");
            uint64_t cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
            printf("  Fault Address (CR2): 0x%lx\n", cr2);
            printf("  Page Fault Flags:\n");
            printf("    Present: %s\n", (frame->err_code & 1) ? "Yes" : "No");
            printf("    Write: %s\n", (frame->err_code & 2) ? "Yes" : "No"); /*Make sure to print these*/
            printf("    User: %s\n", (frame->err_code & 4) ? "Yes" : "No");
            break;
        default:
			/*
				No idea
			*/
            printf("  Type: UNKNOWN EXCEPTION\n");
            break;
    }
    /*
		DUMP MINIAC HUMAGOUES!!!!
	*/
    printf("\n");
    printf("REGISTER DUMP:\n");
    printf("  RIP: 0x%016lx  (Instruction Pointer)\n", frame->rip);
    printf("  RSP: 0x%016lx  (Stack Pointer)\n", frame->rsp);
    printf("  RBP: 0x%016lx  (Base Pointer)\n", frame->rbp);
    printf("  RFLAGS: 0x%016lx\n", frame->rflags);
    printf("  CS: 0x%04lx SS: 0x%04lx\n", 
           frame->cs, frame->ss);
    
    /*
		Also get others
	*/
    uint64_t rax, rbx, rcx, rdx, rsi, rdi;
    __asm__ volatile("mov %%rax, %0" : "=m"(rax));
    __asm__ volatile("mov %%rbx, %0" : "=m"(rbx));
    __asm__ volatile("mov %%rcx, %0" : "=m"(rcx));
    __asm__ volatile("mov %%rdx, %0" : "=m"(rdx));
    __asm__ volatile("mov %%rsi, %0" : "=m"(rsi));
    __asm__ volatile("mov %%rdi, %0" : "=m"(rdi));
    
    printf("  RAX: 0x%016lx  RBX: 0x%016lx\n", rax, rbx);
    printf("  RCX: 0x%016lx  RDX: 0x%016lx\n", rcx, rdx);
    printf("  RSI: 0x%016lx  RDI: 0x%016lx\n", rsi, rdi);
    
    /*
		same for the CRs (control registers)
	*/
    uint64_t cr0, cr3, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    
    printf("\n");
    printf("CONTROL REGISTERS:\n");
    printf("  CR0: 0x%016lx  (System Control)\n", cr0);
    printf("  CR3: 0x%016lx  (Page Directory)\n", cr3);
    printf("  CR4: 0x%016lx  (Extended Features)\n", cr4);
    
    printf("\n");
    printf("CODE DUMP (around RIP):\n");
    uint8_t* code_ptr = (uint8_t*)frame->rip;
    printf("  Address: 0x%lx\n", frame->rip);
    printf("  Bytes: ");
    
    /*
		Dump the code around the RIP
	*/
    for (int i = -8; i < 8; i++) {
        if (i == 0) printf("[");
        printf("%02x ", code_ptr[i]);
        if (i == 0) printf("]");
    }
    printf("\n");
    
    printf("\n");
    printf("STACK DUMP (RSP area):\n");
    uint64_t* stack_ptr = (uint64_t*)frame->rsp;
    printf("  Stack Pointer: 0x%lx\n", frame->rsp);
    /*
		Also for the stack
	*/
    for (int i = 0; i < 8; i++) {
        printf("  RSP+%02d: 0x%016lx", i*8, stack_ptr[i]);
        if (i == 0) printf("  <-- Current RSP");
        printf("\n");
    }
    
    printf("\n");
    printf("STACK TRACE ANALYSIS:\n");
    uint64_t* rbp_ptr = (uint64_t*)frame->rbp;
    for (int i = 0; i < 5 && rbp_ptr; i++) {
        printf("  Frame %d: RBP=0x%lx, Return=0x%lx\n", 
               i, (uint64_t)rbp_ptr, rbp_ptr[1]);
        rbp_ptr = (uint64_t*)rbp_ptr[0];
        if ((uint64_t)rbp_ptr < 0x100000 || (uint64_t)rbp_ptr > 0x1000000) break;
    }
    
    printf("\n");
    printf("MEMORY STATE:\n");
    printf("  Kernel Stack Range: 0x%lx - 0x%lx\n", 
           frame->rsp & ~0xFFF, (frame->rsp & ~0xFFF) + 0x1000);
    printf("  Code Segment: 0x%lx\n", frame->rip & ~0xFFF);
    /*
		state is important too
	*/
    printf("\n");
    printf("SYSTEM STATE:\n");
    printf("  Interrupts: %s\n", (frame->rflags & 0x200) ? "Enabled" : "Disabled");
    printf("  Privilege Level: Ring %lu\n", frame->cs & 3);
    printf("  Paging: %s\n", (cr0 & 0x80000000) ? "Enabled" : "Disabled");
    
    printf("\n");
    printf("=====================================\n");
    printf("         SYSTEM HALTED\n");
    printf("=====================================\n");
    /*
	
		FOR NOW HALT THIS.
		(will add a reboot later)

	*/
    while (1) {
        __asm__ volatile("cli; hlt");
    }
}
