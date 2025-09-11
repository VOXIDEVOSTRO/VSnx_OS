# Dynamic VOSTROX Library Makefile

#	MIT License
	
#	Copyright (c) 2025 Aditya Bansal
	
#	Permission is hereby granted, free of charge, to any person obtaining 
#	a copy of this software and associated documentation files (the "Software"), 
#	to deal in the Software without restriction, including without limitation the 
#	rights to use, copy, modify, merge, publish, distribute, sublicense, 
#	and/or sell copies of the Software, and to permit persons to whom 
#	the Software is furnished to do so, subject to the following conditions:
	
#	The above copyright notice and this permission notice shall be 
#	included in all copies or substantial portions of the Software.
	
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
#	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
#	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

CC = gcc
LD = ld

CFLAGS = -mno-red-zone -fno-stack-protector -fno-pic -fno-pie \
         -O0 -fcf-protection=none \
		 -c -m64 -march=x86-64 -mtune=generic
LDFLAGS = -n -T linker.ld -e _start

# Directories
BUILDDIR = ../build
FINAL_LIBDIR = $(BUILDDIR)/temp/os/modules/sys/libs
LOCAL_BUILDDIR = build

# Auto-discover all .c files recursively
C_SOURCES = $(shell find . -type f -name '*.c')

# Mirror structure in LOCAL and FINAL build dirs
LOCAL_OBJECTS = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.o,$(C_SOURCES))
LOCAL_ELFS    = $(patsubst ./%.c,$(LOCAL_BUILDDIR)/%.elf,$(C_SOURCES))
FINAL_ELFS    = $(patsubst ./%.c,$(FINAL_LIBDIR)/%.elf,$(C_SOURCES))

.PHONY: all clean

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