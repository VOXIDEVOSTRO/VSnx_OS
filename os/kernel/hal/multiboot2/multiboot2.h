#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H
/*
	Header files
*/
#include <stdint.h>
#include "../serial_com/serial.h"
#include "../vga/vga_text/vga_text.h"
#include "../../utilities/utility.h"
/*
	magic numbers
*/
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289
#define MULTIBOOT2_HEADER_MAGIC     0xe85250d6
/*
	All tag types
*/
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
/*
	memory map entries
*/
#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5
/*
	m2 info structure
*/
typedef struct {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed)) multiboot2_info_t;
/*
	m2 tag structure
*/
typedef struct {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) multiboot2_tag_t;
/*
	m2 basic mem info
*/
typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed)) multiboot2_tag_basic_meminfo_t;
/*
	m2 memory map entry structure
*/
typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed)) multiboot2_mmap_entry_t;
/*
	m2 memory map tag structure
*/
typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot2_mmap_entry_t entries[];
} __attribute__((packed)) multiboot2_tag_mmap_t;
/*
	the main memory map sturcture for the entire VSnx
*/
typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} vsnx_memory_region_t;
typedef struct {
    uint32_t region_count;
    uint64_t total_memory;
    uint64_t usable_memory;
    vsnx_memory_region_t regions[256];
} vsnx_memory_map_t;
/*
	Globals
*/
extern vsnx_memory_map_t vsnx_mmap;
/*
	Prototypes
*/
/*
	mmap.c
*/
int multiboot2_parse_memory_map(uint64_t multiboot2_addr);
vsnx_memory_map_t* get_vsnx_memory_map(void);
uint64_t get_total_memory(void);
uint64_t get_usable_memory(void);
#endif