#ifndef GDT_H
#define GDT_H
/*
	Header files
*/
#include <stdint.h>
/*
	Structures
*/
// GDT Entry Structure
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

// TSS Entry Structure (64-bit)
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed)) tss_entry_t;

// GDT Pointer Structure
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

// TSS Structure
typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;    // Stack pointer for ring 0
    uint64_t rsp1;    // Stack pointer for ring 1
    uint64_t rsp2;    // Stack pointer for ring 2
    uint64_t reserved1;
    uint64_t ist1;    // Interrupt Stack Table
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;
/*
	MACROS
*/
// GDT Access Flags
#define GDT_ACCESS_PRESENT     0x80
#define GDT_ACCESS_RING0       0x00
#define GDT_ACCESS_RING3       0x60
#define GDT_ACCESS_EXECUTABLE  0x08
#define GDT_ACCESS_READWRITE   0x02
#define GDT_ACCESS_TSS         0x89

// GDT Granularity Flags
#define GDT_GRAN_64BIT         0x20
#define GDT_GRAN_32BIT         0x40
#define GDT_GRAN_4K            0x80

// Segment Selectors
// Segment Selectors - CORRECTED
#define KERNEL_CODE_SELECTOR   0x08  // Ring 0 (0x08 + 0 = 0x08)
#define KERNEL_DATA_SELECTOR   0x10  // Ring 0 (0x10 + 0 = 0x10)
#define USER_CODE_SELECTOR     0x1B  // Ring 3 (0x18 + 3 = 0x1B)
#define USER_DATA_SELECTOR     0x23  // Ring 3 (0x20 + 3 = 0x23)
#define TSS_SELECTOR           0x28

/*
	Globals
*/
// GDT Table (6 entries: null, kernel code, kernel data, user code, user data, TSS)
extern gdt_entry_t gdt_entries[7];
extern tss_entry_t tss_entry;
extern gdt_ptr_t gdt_ptr;
extern tss_t tss;
/*
	prototypes
*/
void gdt_flush_inline(void);
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_set_tss_entry(int index, uint64_t base, uint32_t limit, uint8_t access, uint8_t gran);
void tss_init(void);
void gdt_init(void);
#endif