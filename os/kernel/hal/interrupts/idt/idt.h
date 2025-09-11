#ifndef IDT_H
#define IDT_H
/*
	header files
*/
#include <stdint.h>
#include "../../../utilities/utility.h"
#include "../../serial_com/serial.h"
#include "../gdt/gdt.h"
/*
	Structs
*/
// IDT Entry Structure (64-bit)
typedef struct {
    uint16_t offset_low;    // Lower 16 bits of handler address
    uint16_t selector;      // Kernel segment selector
    uint8_t  ist;          // Interrupt Stack Table offset
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_mid;   // Middle 16 bits of handler address
    uint32_t offset_high;  // Upper 32 bits of handler address
    uint32_t reserved;     // Reserved, must be zero
} __attribute__((packed)) idt_entry_t;

// IDT Pointer Structure
typedef struct {
    uint16_t limit;        // Size of IDT - 1
    uint64_t base;         // Base address of IDT
} __attribute__((packed)) idt_ptr_t;

// Interrupt Frame Structure (pushed by CPU and our stub)
typedef struct {
    // Pushed by our interrupt stub
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    
    // Pushed by our stub (interrupt number and error code)
    uint64_t int_no;       // Interrupt number
    uint64_t err_code;     // Error code (if applicable)
    
    // Pushed by CPU automatically
    uint64_t rip;          // Instruction pointer
    uint64_t cs;           // Code segment
    uint64_t rflags;       // CPU flags
    uint64_t rsp;          // Stack pointer
    uint64_t ss;           // Stack segment
} __attribute__((packed)) interrupt_frame_t;
/*
	TONS of macros
*/
// IDT Type Attributes
#define IDT_TYPE_INTERRUPT_GATE  0x8E  // 32-bit interrupt gate, ring 0
#define IDT_TYPE_TRAP_GATE       0x8F  // 32-bit trap gate, ring 0
#define IDT_TYPE_TASK_GATE       0x85  // Task gate
#define IDT_TYPE_USER_INT_GATE   0xEE  // 32-bit interrupt gate, ring 3

// Exception Numbers
#define EXCEPTION_DIVIDE_ERROR          0
#define EXCEPTION_DEBUG                 1
#define EXCEPTION_NMI                   2
#define EXCEPTION_BREAKPOINT            3
#define EXCEPTION_OVERFLOW              4
#define EXCEPTION_BOUND_RANGE           5
#define EXCEPTION_INVALID_OPCODE        6
#define EXCEPTION_DEVICE_NOT_AVAILABLE  7
#define EXCEPTION_DOUBLE_FAULT          8
#define EXCEPTION_INVALID_TSS           10
#define EXCEPTION_SEGMENT_NOT_PRESENT   11
#define EXCEPTION_STACK_FAULT           12
#define EXCEPTION_GENERAL_PROTECTION    13
#define EXCEPTION_PAGE_FAULT            14
#define EXCEPTION_FPU_ERROR             16
#define EXCEPTION_ALIGNMENT_CHECK       17
#define EXCEPTION_MACHINE_CHECK         18
#define EXCEPTION_SIMD_FP_EXCEPTION     19

// IRQ Numbers (remapped to 32-47)
#define IRQ_TIMER                0   // IRQ 0  -> INT 32
#define IRQ_KEYBOARD             1   // IRQ 1  -> INT 33
#define IRQ_CASCADE              2   // IRQ 2  -> INT 34 (cascade)
#define IRQ_COM2                 3   // IRQ 3  -> INT 35
#define IRQ_COM1                 4   // IRQ 4  -> INT 36
#define IRQ_LPT2                 5   // IRQ 5  -> INT 37
#define IRQ_FLOPPY               6   // IRQ 6  -> INT 38
#define IRQ_LPT1                 7   // IRQ 7  -> INT 39
#define IRQ_RTC                  8   // IRQ 8  -> INT 40
#define IRQ_FREE1                9   // IRQ 9  -> INT 41
#define IRQ_FREE2                10  // IRQ 10 -> INT 42
#define IRQ_FREE3                11  // IRQ 11 -> INT 43
#define IRQ_MOUSE                12  // IRQ 12 -> INT 44
#define IRQ_FPU                  13  // IRQ 13 -> INT 45
#define IRQ_ATA_PRIMARY          14  // IRQ 14 -> INT 46
#define IRQ_ATA_SECONDARY        15  // IRQ 15 -> INT 47

// PIC Ports
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

// PIC Commands
#define PIC_EOI         0x20  // End of Interrupt

// Global Variables
extern idt_entry_t idt_entries[256];
extern idt_ptr_t idt_ptr;

// Core IDT Functions
void idt_init(void);
void idt_set_entry(int index, uint64_t handler, uint16_t selector, uint8_t flags);

// Interrupt Handlers
void isr_handler(interrupt_frame_t* frame);
void irq_handler(interrupt_frame_t* frame);

// PIC Functions
void pic_send_eoi(uint8_t irq);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

// ISR Function Declarations (0-19: CPU Exceptions)
void isr0(void);   void isr1(void);   void isr2(void);   void isr3(void);
void isr4(void);   void isr5(void);   void isr6(void);   void isr7(void);
void isr8(void);   void isr9(void);   void isr10(void);  void isr11(void);
void isr12(void);  void isr13(void);  void isr14(void);  void isr15(void);
void isr16(void);  void isr17(void);  void isr18(void);  void isr19(void);
void isr20(void);  void isr21(void);  void isr22(void);  void isr23(void);
void isr24(void);  void isr25(void);  void isr26(void);  void isr27(void);
void isr28(void);  void isr29(void);  void isr30(void);  void isr31(void);

// IRQ Function Declarations (32-47: Hardware Interrupts)
void irq0(void);   void irq1(void);   void irq2(void);   void irq3(void);
void irq4(void);   void irq5(void);   void irq6(void);   void irq7(void);
void irq8(void);   void irq9(void);   void irq10(void);  void irq11(void);
void irq12(void);  void irq13(void);  void irq14(void);  void irq15(void);

// Utility Functions
void enable_interrupts(void);
void disable_interrupts(void);
int interrupts_enabled(void);

// Inline utility functions
static inline void cli(void) {
    __asm__ volatile("cli");
}

static inline void sti(void) {
    __asm__ volatile("sti");
}

static inline void hlt(void) {
    __asm__ volatile("hlt");
}

// Exception names for debugging
extern const char* exception_names[32];
/*
	prototypes
*/
void idt_set_entry(int index, uint64_t handler, uint16_t selector, uint8_t flags);
void pic_init(void);
void idt_init(void);

#endif
