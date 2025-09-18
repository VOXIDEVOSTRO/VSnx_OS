#ifndef ELF_H
#define ELF_H
/*
	Very basic of a ELF loader,
	also tied to our process
*/
/*
	header files
*/
#include <stdint.h>
#include "../fat32/gristle.h" // for fat32
#include "../hal/mem/u_mem/u_mem.h"
#include "../utilities/utility.h"
/*
	Standard ELF header
*/
typedef struct {
    uint8_t  e_ident[16];    // ELF identification
    uint16_t e_type;         // Object file type
    uint16_t e_machine;      // Architecture Could be anything
    uint32_t e_version;      // Object file version
    uint64_t e_entry;        // Entry point virtual address
    uint64_t e_phoff;        // Program header table offset
    uint64_t e_shoff;        // Section header table offset
    uint32_t e_flags;        // Processor-specific flags
    uint16_t e_ehsize;       // ELF header size (sizeof elf64_hdr_t???)
    uint16_t e_phentsize;    // Program header entry size
    uint16_t e_phnum;        // Program header entry count
    uint16_t e_shentsize;    // Section header entry size
    uint16_t e_shnum;        // Section header entry count
    uint16_t e_shstrndx;     // Section header string table index (for strings)
} __attribute__((packed)) elf64_hdr_t;
/*
	Program header
*/
typedef struct {
    uint32_t p_type;         // Segment type
    uint32_t p_flags;        // Segment flags
    uint64_t p_offset;       // Segment file offset
    uint64_t p_vaddr;        // Segment virtual address (VMM will handle this)
    uint64_t p_paddr;        // Segment physical address (we using VMM for now)
    uint64_t p_filesz;       // Segment size in file
    uint64_t p_memsz;        // Segment size in memory (important because of elf loader)
    uint64_t p_align;        // Segment alignment
} __attribute__((packed)) elf64_phdr_t;
/*
	Normal constants
*/
#define ELF_MAGIC    0x464C457F  // "\x7FELF" Which is a standard ELF magic
#define ET_EXEC      2           // Executable file
#define EM_X86_64    62          // AMD64/x86-64 (as our OS)
#define PT_LOAD      1           // Loadable segment
/*
	Prototypes
*/
uint64_t elf_parse(const char* filename);
uint64_t elf_load_segment(elf64_phdr_t* phdr, int fd, uint64_t elf_entry);
#endif
