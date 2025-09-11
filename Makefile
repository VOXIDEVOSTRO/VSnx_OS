# Directories
BUILDDIR = build
TEMPDIR = $(BUILDDIR)/temp
OVERLAYDIR = $(TEMPDIR)/overlay

# Files
INSTALLER_ISO = installer/vostrox_86.iso
OUTPUT_ISO = $(BUILDDIR)/vostrox.iso
INSTALLER_SCRIPT = installer/installer.sh

.PHONY: all clean iso os

all: libs os iso

# Create build directory structure
$(BUILDDIR):
	mkdir -p $(BUILDDIR)
	mkdir -p $(TEMPDIR)

# Build OS and then create combined ISO
iso: os $(BUILDDIR)
	# Extract the ISO content to temp directory
	7z x -o$(TEMPDIR) $(INSTALLER_ISO)
	
	# Create os directory and copy OS files
	mkdir -p $(TEMPDIR)/os
	cp -r os/$(BUILDDIR)/os/* $(TEMPDIR)/os/

#	cp -r hello.bmp $(TEMPDIR)/os/
	
	# Copy GRUB directory to the root of the ISO
	mkdir -p $(TEMPDIR)/boot
	cp -r os/boot/grub $(TEMPDIR)
	
	# Create overlay directory structure
	mkdir -p $(OVERLAYDIR)/etc/init.d
	mkdir -p $(OVERLAYDIR)/etc/runlevels/default
	
	# Copy installer script to overlay
	cp $(INSTALLER_SCRIPT) $(OVERLAYDIR)/installer.sh
	chmod +x $(OVERLAYDIR)/installer.sh
	
	# Create init.d service script
	echo '#!/sbin/openrc-run' > $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo 'description="VOSTROX OS Installer"' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo 'depend() {' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '    need net' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '}' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo 'start() {' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '    ebegin "Starting VOSTROX OS Installer"' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '    /overlay/installer.sh &' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '    eend $$?' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	echo '}' >> $(OVERLAYDIR)/etc/init.d/vostrox-installer
	chmod +x $(OVERLAYDIR)/etc/init.d/vostrox-installer
	
	# Enable service in default runlevel
	ln -sf /etc/init.d/vostrox-installer $(OVERLAYDIR)/etc/runlevels/default/vostrox-installer
	
	# Create overlay squashfs image
	mksquashfs $(OVERLAYDIR) $(TEMPDIR)/overlay.squashfs -comp xz
	
	# Update syslinux.cfg to include overlay
	sed -i 's/quiet/quiet modloop=\/boot\/modloop-lts overlay_root=\/overlay/' $(TEMPDIR)/boot/syslinux/syslinux.cfg
	
	# Copy installer script to root of ISO (for backward compatibility)
	cp $(INSTALLER_SCRIPT) $(TEMPDIR)/installer.sh
	chmod +x $(TEMPDIR)/installer.sh

	# Copy font files to the ISO
#	cp -r fonts $(TEMPDIR)/os/modules/sys/
	
	# Create a new ISO with mkisofs
	mkisofs -o $(OUTPUT_ISO) \
		-b boot/syslinux/isolinux.bin \
		-c boot/syslinux/boot.cat \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-J -R -V "VOSTROX" \
		$(TEMPDIR)
	
	# Make the ISO bootable from USB drives using isohybrid
	isohybrid $(OUTPUT_ISO) || echo "isohybrid not available, ISO may not boot from USB"
	
	@echo "Combined ISO created at $(OUTPUT_ISO)"

# Build OS
os:
	$(MAKE) -C os
	$(MAKE) -C libs
	$(MAKE) -C apps
	$(MAKE) -C drivers
#	$(MAKE) -C modules

# Clean build artifactss
clean:
	rm -rf $(BUILDDIR)
	$(MAKE) -C os clean
	$(MAKE) -C apps clean
	$(MAKE) -C libs clean
	$(MAKE) -C drivers clean
#	$(MAKE) -C modules clean
