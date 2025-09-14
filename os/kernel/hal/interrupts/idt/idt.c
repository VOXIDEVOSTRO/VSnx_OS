/*
	Header files
*/
#include "idt.h"
/*
    ISR Stubs (declared in assembly or inline)
*/
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);

/*
    IRQ Stubs
*/
extern void irq0(void);  extern void irq1(void);  extern void irq2(void);  extern void irq3(void);
extern void irq4(void);  extern void irq5(void);  extern void irq6(void);  extern void irq7(void);
extern void irq8(void);  extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);

/*
    Set IDT Entry
*/
void idt_set_entry(int index, uint64_t handler, uint16_t selector, uint8_t flags) {
    idt_entries[index].offset_low  = handler & 0xFFFF;
    idt_entries[index].selector    = selector;
    idt_entries[index].ist         = 0;
    idt_entries[index].type_attr   = flags;
    idt_entries[index].offset_mid  = (handler >> 16) & 0xFFFF;
    idt_entries[index].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt_entries[index].reserved    = 0;
}

/*
    Initialize PIC
*/
void pic_init(void) {
    // ICW1: Initialize PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2: Remap IRQs
    outb(0x21, 0x20);  // Master PIC starts at 32
    outb(0xA1, 0x28);  // Slave PIC starts at 40
    
    // ICW3: Setup cascade
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    // ICW4: 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Mask all IRQs except cascade
    outb(0x21, 0xFE);  // Enable IRQ0 (timer)
    outb(0xA1, 0xFF);
    
	#ifdef DEBUG
    printf("PIC: Programmable Interrupt Controller initialized\n");
	#endif
}

/*
    IDT Initialization
*/
void idt_init(void) {
	#ifdef DEBUG
    printf("IDT: Initializing Interrupt Descriptor Table...\n");
	#endif
    
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint64_t)&idt_entries;
    
    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, 0, 0, 0);
    }
    
    // Setup ISRs (Exceptions)
    idt_set_entry(0,  (uint64_t)isr0,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(1,  (uint64_t)isr1,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(2,  (uint64_t)isr2,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(3,  (uint64_t)isr3,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(4,  (uint64_t)isr4,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(5,  (uint64_t)isr5,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(6,  (uint64_t)isr6,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(7,  (uint64_t)isr7,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(8,  (uint64_t)isr8,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(9,  (uint64_t)isr9,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(10, (uint64_t)isr10, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(11, (uint64_t)isr11, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(12, (uint64_t)isr12, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(13, (uint64_t)isr13, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(14, (uint64_t)isr14, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(15, (uint64_t)isr15, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(16, (uint64_t)isr16, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(17, (uint64_t)isr17, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(18, (uint64_t)isr18, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(19, (uint64_t)isr19, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    
    // Setup IRQs (Hardware Interrupts)
    idt_set_entry(32, (uint64_t)irq0,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(33, (uint64_t)irq1,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(34, (uint64_t)irq2,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(35, (uint64_t)irq3,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(36, (uint64_t)irq4,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(37, (uint64_t)irq5,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(38, (uint64_t)irq6,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(39, (uint64_t)irq7,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(40, (uint64_t)irq8,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(41, (uint64_t)irq9,  KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(42, (uint64_t)irq10, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(43, (uint64_t)irq11, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(44, (uint64_t)irq12, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(45, (uint64_t)irq13, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(46, (uint64_t)irq14, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    idt_set_entry(47, (uint64_t)irq15, KERNEL_CODE_SELECTOR, IDT_TYPE_INTERRUPT_GATE);
    
    // Initialize PIC
    pic_init();
    
    // Load IDT
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
    
    // Enable interrupts
    __asm__ volatile("sti");
    
	#ifdef DEBUG
    printf("IDT: Interrupt Descriptor Table initialized\n");
    printf("IDT: Interrupts enabled\n");
	#endif
}

// Exception name table for debugging
const char* exception_names[32] = {
    "Division Error",
    "Debug Exception",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", "Reserved"
};
/*
	Declare the globals here
*/
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;
/*
    ISR Stubs using inline assembly
*/

#define ISR_STUB(num) \
    void isr##num(void) { \
        __asm__ volatile( \
            "pushq $0\n\t" \
            "pushq $" #num "\n\t" \
            "jmp isr_common_stub\n\t" \
        ); \
    }

#define ISR_STUB_ERR(num) \
    void isr##num(void) { \
        __asm__ volatile( \
            "pushq $" #num "\n\t" \
            "jmp isr_common_stub\n\t" \
        ); \
    }

#define IRQ_STUB(num, int_num) \
    void irq##num(void) { \
        __asm__ volatile( \
            "pushq $0\n\t" \
            "pushq $" #int_num "\n\t" \
            "jmp irq_common_stub\n\t" \
        ); \
    }

// Generate IRQ stubs with correct interrupt numbers
IRQ_STUB(0, 32)   IRQ_STUB(1, 33)   IRQ_STUB(2, 34)   IRQ_STUB(3, 35)
IRQ_STUB(4, 36)   IRQ_STUB(5, 37)   IRQ_STUB(6, 38)   IRQ_STUB(7, 39)
IRQ_STUB(8, 40)   IRQ_STUB(9, 41)   IRQ_STUB(10, 42)  IRQ_STUB(11, 43)
IRQ_STUB(12, 44)  IRQ_STUB(13, 45)  IRQ_STUB(14, 46)  IRQ_STUB(15, 47)

// Generate all stubs
ISR_STUB(0)  ISR_STUB(1)  ISR_STUB(2)  ISR_STUB(3)
ISR_STUB(4)  ISR_STUB(5)  ISR_STUB(6)  ISR_STUB(7)
ISR_STUB_ERR(8)  ISR_STUB(9)  ISR_STUB_ERR(10) ISR_STUB_ERR(11)
ISR_STUB_ERR(12) ISR_STUB_ERR(13) ISR_STUB_ERR(14) ISR_STUB(15)
ISR_STUB(16) ISR_STUB(17) ISR_STUB(18) ISR_STUB(19)

/*
    Common interrupt stubs
*/
/*
    Common interrupt stubs - ROBUST VERSION
*/
__asm__(
    "isr_common_stub:\n\t"
    "pushq %rax\n\t"
    "pushq %rbx\n\t"
    "pushq %rcx\n\t"
    "pushq %rdx\n\t"
    "pushq %rsi\n\t"
    "pushq %rdi\n\t"
    "pushq %rbp\n\t"
    "pushq %r8\n\t"
    "pushq %r9\n\t"
    "pushq %r10\n\t"
    "pushq %r11\n\t"
    "pushq %r12\n\t"
    "pushq %r13\n\t"
    "pushq %r14\n\t"
    "pushq %r15\n\t"
    "movq %rsp, %rdi\n\t"      // Pass stack pointer as argument
    "call isr_handler\n\t"
    "popq %r15\n\t"
    "popq %r14\n\t"
    "popq %r13\n\t"
    "popq %r12\n\t"
    "popq %r11\n\t"
    "popq %r10\n\t"
    "popq %r9\n\t"
    "popq %r8\n\t"
    "popq %rbp\n\t"
    "popq %rdi\n\t"
    "popq %rsi\n\t"
    "popq %rdx\n\t"
    "popq %rcx\n\t"
    "popq %rbx\n\t"
    "popq %rax\n\t"
    "addq $16, %rsp\n\t"       // Remove error code and interrupt number
    "iretq\n\t"
);

__asm__(
    "irq_common_stub:\n\t"
    "pushq %rax\n\t"
    "pushq %rbx\n\t"
    "pushq %rcx\n\t"
    "pushq %rdx\n\t"
    "pushq %rsi\n\t"
    "pushq %rdi\n\t"
    "pushq %rbp\n\t"
    "pushq %r8\n\t"
    "pushq %r9\n\t"
    "pushq %r10\n\t"
    "pushq %r11\n\t"
    "pushq %r12\n\t"
    "pushq %r13\n\t"
    "pushq %r14\n\t"
    "pushq %r15\n\t"
    "movq %rsp, %rdi\n\t"      // Pass stack pointer as argument
    "call irq_handler\n\t"
    "popq %r15\n\t"
    "popq %r14\n\t"
    "popq %r13\n\t"
    "popq %r12\n\t"
    "popq %r11\n\t"
    "popq %r10\n\t"
    "popq %r9\n\t"
    "popq %r8\n\t"
    "popq %rbp\n\t"
    "popq %rdi\n\t"
    "popq %rsi\n\t"
    "popq %rdx\n\t"
    "popq %rcx\n\t"
    "popq %rbx\n\t"
    "popq %rax\n\t"
    "addq $16, %rsp\n\t"       // Remove error code and interrupt number
    "iretq\n\t"
);