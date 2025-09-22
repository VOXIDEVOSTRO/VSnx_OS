/*
	Inclueds or header... whatever you like
*/
#include "vbox.h"
/*

	Main check for PCI

*/
static int vboxsvga_pci_present(void) {
    int found = 0;
	/*
		scan bus
	*/
    int count = pci_scan_bus();
    (void)count;

    unsigned long probes = 0;
	/*
		Timeout
		if yes it means no VBOXSVGA
	*/
	#define PCI_SCAN_TIMEOUT  (256*32*8)
	/*
		Scan up
	*/
    for (uint8_t bus = 0; bus < 256; ++bus) {
        for (uint8_t dev = 0; dev < 32; ++dev) {
            for (uint8_t fn = 0; fn < 8; ++fn) {
				/*
					Handle timeout
				*/
                if (++probes > PCI_SCAN_TIMEOUT) {
                    #ifdef DEBUG
                    printf("[vboxsvga] Timeout scanning PCI, no device found\n");
                    #endif
                    return 0;
                }
				/*
					Read the vendor
				*/
                uint16_t vendor = pci_config_read16(bus, dev, fn, PCI_VENDOR_ID);
                if (vendor == 0xFFFF/*invalid*/) continue;
				/*
					Read class code
				*/
                uint8_t class_code = pci_config_read8(bus, dev, fn, PCI_CLASS_CODE);
				/*
					As well as vendor
				*/
                uint16_t device_id = pci_config_read16(bus, dev, fn, PCI_DEVICE_ID);
				/*
					Find up virtualbox
				*/
                if (vendor == VENDOR_VIRTUALBOX && class_code == PCI_CLASS_DISPLAY) {
                    #ifdef DEBUG
                    printf("[vboxsvga] Found VirtualBox display device %02x:%02x.%u (dev=%04x)\n", bus, dev, fn, device_id);
                    #endif
					/*
						Good boy
					*/
                    found = 1;
                    goto done;
                }
            }
        }
    }

done:
    if (!found) {
		/*
			None found
		*/
        #ifdef DEBUG
        printf("[vboxsvga] No VirtualBox display device found\n");
        #endif
    }
	/*
		Found
	*/
    return found;
}

int vboxsvga_spawn_driver(void) {
	/*
		Is hookorder present?
	*/
    if (hookorder_count() == 0) {
        int loaded = hookorder_load(NULL);
        if (loaded < 0) {
            #ifdef DEBUG
            printf("[vboxsvga] hookorder load failed (%d)\n", loaded);
            #endif
            return -1;
        }
    }

	/*
		check for VBOX
	*/
    if (!vboxsvga_pci_present()) {
		#ifdef DEBUG
        printf("[vboxsvga] No VirtualBox display detected; skipping\n");
		#endif
        return -2;
    }

	/*
		Check for entry
	*/
    const hookorder_entry_t* entry = hookorder_find_exact("GPU", "VBOX");
    if (!entry) {
		/*
			wildcard
		*/
        entry = hookorder_find_best("GPU", "VBOX");
    }
    if (!entry) {
        #ifdef DEBUG
        printf("[vboxsvga] No hookorder entry for CLASS:GPU:VBOX\n");
        #endif
        return -3;
    }
	/*
		Spawn the driver!
	*/
    process_t* proc = NULL;
	#ifdef DEBUG
	printf("[vboxsvga] spawning the driver...\n");
	#endif
    int rc = hookorder_spawn_for("GPU", "VBOX", &proc);
    if (rc < 0 || !proc) {
        #ifdef DEBUG
        printf("[vboxsvga] Failed to spawn VBOX driver (rc=%d)\n", rc);
        #endif
        return -4;
    }

    #ifdef DEBUG
    printf("[vboxsvga] Spawned driver: %s (pid=%u)\n", entry->path, proc->pid);
    #endif
	/*
		Done
	*/
    return 0;
}
