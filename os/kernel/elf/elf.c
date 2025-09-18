/*
	Header files duh...
*/
#include "elf.h"
/*
	ELF parse.
	This will just find and open and parse the ELF binary. ez
*/
uint64_t elf_parse(const char* filename) {
    #ifdef DEBUG
    printf("ELF: Loading %s\n", filename);
    #endif
    /*
		open the file DUH...
	*/
    int rerrno = 0; // Keep it NULL
    int fd = fat_open(filename, O_RDONLY, 0, &rerrno);
    if (fd < 0) {
        #ifdef DEBUG
        printf("ELF: Failed to open %s\n", filename);
        #endif
        return 0;
    }
    /*
		Read the header
	*/
    elf64_hdr_t elf_hdr;
    if (fat_read(fd, &elf_hdr, sizeof(elf_hdr)/*Told ya*/, &rerrno) != sizeof(elf_hdr)) {
        #ifdef DEBUG
        printf("ELF: Failed to read header\n");
        #endif
        fat_close(fd, &rerrno);
        return 0;
    }
    /*
		Validate the magic
		The magic is [0x7F]ELF
		or in raw bytes its: 0x464C457F 
	*/
    if (*(uint32_t*)elf_hdr.e_ident != ELF_MAGIC) {
        #ifdef DEBUG
        printf("ELF: Invalid magic number\n");
        #endif
        fat_close(fd, &rerrno);
        return 0;
    }
    /*
		Also validate the artitecture
		we are in a AMD64/x86-64
	*/
    if (elf_hdr.e_machine != EM_X86_64 || elf_hdr.e_type != ET_EXEC) {
        #ifdef DEBUG
        printf("ELF: Unsupported format\n");
        #endif
        fat_close(fd, &rerrno);
        return 0;
    }
    
    #ifdef DEBUG
    printf("ELF: Valid x86-64 executable or AMD64, entry=0x%lx\n", elf_hdr.e_entry);
    #endif
    
    uint64_t actual_entry_point = 0;
    
    /*
		Now load the segments into memory and find the entry point BECAUSE VMM
	*/
    for (int i = 0; i < elf_hdr.e_phnum; i++) {
        elf64_phdr_t phdr;
        
        fat_lseek(fd, elf_hdr.e_phoff + i * sizeof(phdr), SEEK_SET, &rerrno);
        fat_read(fd, &phdr, sizeof(phdr), &rerrno);
        
        if (phdr.p_type == PT_LOAD) {
            uint64_t loaded_entry = elf_load_segment(&phdr, fd, elf_hdr.e_entry);
            if (loaded_entry == 0) {
                #ifdef DEBUG
                printf("ELF: Failed to load segment %d\n", i);
                #endif
                fat_close(fd, &rerrno);
                return 0;
            }
            
            /*
				Any entry point?
			*/
            if (elf_hdr.e_entry >= phdr.p_vaddr && 
                elf_hdr.e_entry < phdr.p_vaddr + phdr.p_memsz) {
                actual_entry_point = loaded_entry;
                #ifdef DEBUG
                printf("ELF: Entry point found in segment %d at 0x%lx\n", i, actual_entry_point);
                #endif
            }
        }
    }
    
	/*
		Close the file
	*/
    fat_close(fd, &rerrno);
    
    if (actual_entry_point == 0) {
        #ifdef DEBUG
        printf("ELF: Entry point not found in any loaded segment, some thing is wrong\n");
        #endif
        return 0;
    }
    
    #ifdef DEBUG
    printf("ELF: Loaded successfully, entry=0x%lx\n", actual_entry_point);
    #endif
	/*
		if any entry point is found, return it
	*/
    return actual_entry_point;
}
/*
	This will load the segments into memory and find the entry point BECAUSE VMM
*/
uint64_t elf_load_segment(elf64_phdr_t* phdr, int fd, uint64_t elf_entry) {
	#ifdef DEBUG
    printf("ELF: Loading segment vaddr=0x%lx size=%lu\n", phdr->p_vaddr, phdr->p_memsz);
	#endif
    
    /*
		Alloc some memory via VMM.
		because VMM is user memory.
		so that user doesnt cause pagefaults
		or segmentation faults?
	*/
    void* segment_mem = umalloc(phdr->p_memsz);
    if (!segment_mem) {
        #ifdef DEBUG
        printf("ELF: VMM allocation failed for segment\n");
        #endif
        return 0;
    }
    
    #ifdef DEBUG
    printf("ELF: VMM allocated segment at 0x%lx\n", (uint64_t)segment_mem);
    #endif
    
    int rerrno = 0; // NULLIFY
    fat_lseek(fd, phdr->p_offset, SEEK_SET, &rerrno);
    
	/*
		read segment data to be loaded in the VMM
	*/
    if (fat_read(fd, segment_mem, phdr->p_filesz, &rerrno) != phdr->p_filesz) {
        #ifdef DEBUG
        printf("ELF: Failed to read segment data\n");
        #endif
        ufree(segment_mem);  // Clean up VMM allocation
        return 0;
    }
    
    /*
		Zero out the BSS
	*/
    if (phdr->p_memsz > phdr->p_filesz) {
        umemset((char*)segment_mem + phdr->p_filesz, 0, 
                phdr->p_memsz - phdr->p_filesz);
    }
    
    #ifdef DEBUG
    printf("ELF: Segment loaded via VMM at 0x%lx\n", (uint64_t)segment_mem);
    #endif
    
    /*
		NOW properly calculate the entry point BECAUSE VMM
		and this important becuase to execute we will load
		the elf and then because VMM can place anywhere in
		the memory, we will calculate the entry point and
		pass it as the thread pointer. then the schedular
		can easily execute the thread JUST via the entry
		point and do stuff. (also we would need heap alloc
		for the program)
	*/
    if (elf_entry >= phdr->p_vaddr && elf_entry < phdr->p_vaddr + phdr->p_memsz) {
	    /*
			Now calculate the entry point
		*/
        uint64_t offset = elf_entry - phdr->p_vaddr;
        uint64_t actual_entry = (uint64_t)segment_mem + offset;
        
        #ifdef DEBUG
        printf("ELF: Entry point mapped from 0x%lx to 0x%lx\n", elf_entry, actual_entry);
        #endif
        return actual_entry;
    }
    
    return 1;  // Success but no entry point in this segment
	/*
		This will be loop called for all segments
	*/
}
