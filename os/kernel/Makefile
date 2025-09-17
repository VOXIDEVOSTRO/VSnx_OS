ASM = nasm
CC = gcc
LD = ld

# boring Flags
KERNEL_ASMFLAGS = -f elf64
KERNEL_CFLAGS = -ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx \
         -fno-stack-protector -fno-pic -fno-pie -O2 -Wall -Wextra -c
KERNEL_LDFLAGS = -n -T linker.ld -nostdlib


BUILDDIR = ../build
OSDIR = $(BUILDDIR)/os

# Find up all sources recursively
KERNEL_C_SOURCES = $(shell find . -name "*.c" -type f)
KERNEL_ASM_SOURCES = ../boot/header.asm
CFG_FILES = $(shell find . -name "*.cfg" -type f)

KERNEL_C_OBJECTS = $(patsubst %.c,$(BUILDDIR)/kernel/%.o,$(KERNEL_C_SOURCES))
KERNEL_ASM_OBJECTS = $(patsubst ../boot/%.asm,$(BUILDDIR)/%.o,$(KERNEL_ASM_SOURCES))

KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

KERNEL_BIN = $(BUILDDIR)/kernel.bin

.PHONY: all clean

all: $(KERNEL_BIN) copy_cfgs

$(BUILDDIR)/kernel:
	mkdir -p $(BUILDDIR)/kernel
	mkdir -p $(BUILDDIR)/kernel/hal

# Compile up
$(BUILDDIR)/kernel/%.o: %.c | $(BUILDDIR)/kernel
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) $< -o $@

$(BUILDDIR)/%.o: ../boot/%.asm | $(BUILDDIR)/kernel
	$(ASM) $(KERNEL_ASMFLAGS) $< -o $@

$(KERNEL_BIN): $(KERNEL_OBJECTS) linker.ld
	$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJECTS)
	mkdir -p $(OSDIR)/kernel
	cp $@ $(OSDIR)/kernel/kernel.bin

copy_cfgs:
	mkdir -p $(OSDIR)/kernel/configs
	for f in $(CFG_FILES); do \
		cp $$f $(OSDIR)/kernel/configs/; \
	done

clean:
	rm -rf $(BUILDDIR)/kernel $(KERNEL_BIN)
