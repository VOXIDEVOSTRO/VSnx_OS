# Kernel Makefile for VSnx OS

# Compilers and tools
ASM = nasm
CC = gcc
LD = ld

# Flags
KERNEL_ASMFLAGS = -f elf64
KERNEL_CFLAGS = -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx \
         -fno-stack-protector -fno-pic -fno-pie -O2 -Wall -Wextra -c
KERNEL_LDFLAGS = -n -T linker.ld -nostdlib

# Directories
BUILDDIR = ../build
OSDIR = $(BUILDDIR)/os

# Find all C files recursively in kernel directory and subdirectories
KERNEL_C_SOURCES = $(shell find . -name "*.c" -type f)
KERNEL_ASM_SOURCES = ../boot/header.asm

# Generate object files
KERNEL_C_OBJECTS = $(patsubst %.c,$(BUILDDIR)/kernel/%.o,$(KERNEL_C_SOURCES))
KERNEL_ASM_OBJECTS = $(patsubst ../boot/%.asm,$(BUILDDIR)/%.o,$(KERNEL_ASM_SOURCES))

# All objects
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

# Output binary
KERNEL_BIN = $(BUILDDIR)/kernel.bin

.PHONY: all clean

all: $(KERNEL_BIN)

# Create build directories
$(BUILDDIR)/kernel:
	mkdir -p $(BUILDDIR)/kernel
	mkdir -p $(BUILDDIR)/kernel/hal

# Compile C files to objects
$(BUILDDIR)/kernel/%.o: %.c | $(BUILDDIR)/kernel
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) $< -o $@

# Compile ASM files
$(BUILDDIR)/%.o: ../boot/%.asm | $(BUILDDIR)/kernel
	$(ASM) $(KERNEL_ASMFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KERNEL_OBJECTS) linker.ld
	$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJECTS)
	mkdir -p $(OSDIR)/kernel
	cp $@ $(OSDIR)/kernel/kernel.bin

clean:
	rm -rf $(BUILDDIR)/kernel $(KERNEL_BIN)
