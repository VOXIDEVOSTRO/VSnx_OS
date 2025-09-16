CC = gcc
LD = ld

CFLAGS = -mno-red-zone -fno-stack-protector \
         -O2 -fcf-protection=none -nostdlib -nostartfiles -static \
         -ffreestanding -mcmodel=large \
         -c -m64 -march=x86-64 -mtune=generic
LDFLAGS = -n -T linker.ld -e _start

# Directories
BUILDDIR = ../build
FINAL_LIBDIR = $(BUILDDIR)/temp/os/modules/sys/drivers
LOCAL_BUILDDIR = build

# Auto-discover all .c files recursively
C_SOURCES = $(shell find . -type f -name '*.c')

# Mirror structure in LOCAL and FINAL build dirs
LOCAL_OBJECTS = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.o,$(C_SOURCES))
LOCAL_ELFS    = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.elf,$(C_SOURCES))
FINAL_ELFS    = $(patsubst ./%.c,$(FINAL_LIBDIR)/%.elf,$(C_SOURCES))

.PHONY: all clean

# Add fat_driver to the all target
all: $(FINAL_ELFS)

# Create local build directory
$(LOCAL_BUILDDIR):
	mkdir -p $(LOCAL_BUILDDIR)

# Create final libs directory
$(FINAL_LIBDIR):
	mkdir -p $(FINAL_LIBDIR)

# Compile .c to .o locally
$(LOCAL_BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Link .o to .elf locally
$(LOCAL_BUILDDIR)/%.elf: $(LOCAL_BUILDDIR)/%.o
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $<
	@echo "Built library: $@"

# Copy .elf to final build directory
$(FINAL_LIBDIR)/%.elf: $(LOCAL_BUILDDIR)/%.elf
	@mkdir -p $(dir $@)
	cp $< $@
	@echo "Copied to final: $@"

clean:
	rm -rf $(LOCAL_BUILDDIR)
	rm -rf $(FINAL_LIBDIR)