CC = gcc
LD = ld

CFLAGS = -mno-red-zone -fno-stack-protector \
         -O2 -fcf-protection=none -nostdlib -nostartfiles -static \
         -ffreestanding -mcmodel=large \
         -c -m64 -march=x86-64 -mtune=generic
LDFLAGS = -n -T linker.ld -e _start

BUILDDIR = ../build
FINAL_LIBDIR = $(BUILDDIR)/temp/os/modules/apps
LOCAL_BUILDDIR = build

# Recursively discover all .c files
C_SOURCES = $(shell find . -type f -name '*.c')

# Mirror structure for .o and .elf
LOCAL_OBJECTS = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.o,$(C_SOURCES))
LOCAL_ELFS    = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.elf,$(C_SOURCES))
FINAL_ELFS    = $(patsubst ./%.c,$(FINAL_LIBDIR)/%.elf,$(C_SOURCES))


.PHONY: all clean

all: $(FINAL_ELFS)

# Create build directory structure
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile .c to .o
$(LOCAL_BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

# Link .o to .elf
$(LOCAL_BUILDDIR)/%.elf: $(LOCAL_BUILDDIR)/%.o
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $<
	@echo "Built: $@"

# Copy .elf directly to final lib dir
$(FINAL_LIBDIR)/%.elf: $(LOCAL_BUILDDIR)/%.elf
	@mkdir -p $(dir $@)
	cp $< $@
	@echo "Deployed: $@"

clean:
	rm -rf $(LOCAL_BUILDDIR)
	rm -rf $(FINAL_LIBDIR)