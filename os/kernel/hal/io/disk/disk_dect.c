/*
	Header files
*/
#include "disk.h"
#include "ideata.h"
#include "ahci.h"
#include "nvme.h"
#include "scsi.h"
/*
	Known table
*/
static const controller_id_t known_controllers[] = {
    // Intel IDE Controllers
    {0x8086, 0x7010, DISK_TYPE_IDE_ATA, "Intel PIIX3 IDE"},
    {0x8086, 0x7111, DISK_TYPE_IDE_ATA, "Intel PIIX4 IDE"},
    {0x8086, 0x2411, DISK_TYPE_IDE_ATA, "Intel ICH IDE"},
    {0x8086, 0x2421, DISK_TYPE_IDE_ATA, "Intel ICH0 IDE"},
    
    // Intel SATA/AHCI Controllers
    {0x8086, 0x2652, DISK_TYPE_AHCI, "Intel ICH6 SATA"},
    {0x8086, 0x2653, DISK_TYPE_AHCI, "Intel ICH6 AHCI"},
    {0x8086, 0x27c1, DISK_TYPE_AHCI, "Intel ICH7 SATA"},
    {0x8086, 0x27c5, DISK_TYPE_AHCI, "Intel ICH7 AHCI"},
    {0x8086, 0x2829, DISK_TYPE_AHCI, "Intel ICH8 AHCI"},
    {0x8086, 0x2922, DISK_TYPE_AHCI, "Intel ICH9 AHCI"},
    
    // Intel NVMe Controllers
    {0x8086, 0x0953, DISK_TYPE_NVME, "Intel NVMe SSD"},
    {0x8086, 0x0a53, DISK_TYPE_NVME, "Intel NVMe SSD"},
    {0x8086, 0x0a54, DISK_TYPE_NVME, "Intel NVMe SSD"},
    
    // VIA Controllers
    {0x1106, 0x0571, DISK_TYPE_IDE_ATA, "VIA IDE"},
    {0x1106, 0x3149, DISK_TYPE_SATA, "VIA SATA"},
    
    // Samsung NVMe
    {0x144d, 0xa801, DISK_TYPE_NVME, "Samsung NVMe SSD"},
    {0x144d, 0xa802, DISK_TYPE_NVME, "Samsung NVMe SSD"},
    {0x144d, 0xa804, DISK_TYPE_NVME, "Samsung NVMe SSD"},
    
    // Western Digital NVMe
    {0x1b96, 0x1001, DISK_TYPE_NVME, "WD Black NVMe"},
    
    // Micron NVMe
    {0x1344, 0x5405, DISK_TYPE_NVME, "Micron NVMe"},
    
    // VMware/VirtualBox Controllers
    {0x15ad, 0x1976, DISK_TYPE_AHCI, "VMware SATA"},
    {0x80ee, 0x4006, DISK_TYPE_SATA, "VirtualBox SATA"},
    
    // End marker
    {0x0000, 0x0000, DISK_TYPE_UNKNOWN, NULL} // generic controller
};

/*
    Identify controller by vendor/device ID
*/
static const controller_id_t* identify_controller(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; known_controllers[i].name != NULL; i++) {
        if (known_controllers[i].vendor_id == vendor_id && 
            known_controllers[i].device_id == device_id) {
            return &known_controllers[i];
        }
    }
    return NULL;
}

/*
    Identify controller by PCI class/subclass
*/
static disk_type_t identify_by_class(uint8_t class_code, uint8_t subclass, uint8_t prog_if) {
    if (class_code == PCI_CLASS_STORAGE) {
        switch (subclass) {
            case PCI_SUBCLASS_IDE:
                return DISK_TYPE_IDE_ATA;
            case PCI_SUBCLASS_SATA:
                if (prog_if == 0x01) return DISK_TYPE_AHCI;
                return DISK_TYPE_SATA;
            case PCI_SUBCLASS_NVME:
                return DISK_TYPE_NVME;
            case 0x00: // SCSI
                return DISK_TYPE_SCSI;
            case 0x05: // ATA
                return DISK_TYPE_IDE_ATA;
            default:
                return DISK_TYPE_UNKNOWN;
        }
    }
    return DISK_TYPE_UNKNOWN;
}

/*
    Get controller type name
	Or just store in this case
*/
static const char* get_controller_type_name(disk_type_t type) {
    switch (type) {
        case DISK_TYPE_IDE_ATA: return "IDE/ATA";
        case DISK_TYPE_IDE_ATAPI: return "IDE/ATAPI";
        case DISK_TYPE_SATA: return "SATA";
        case DISK_TYPE_AHCI: return "AHCI";
        case DISK_TYPE_NVME: return "NVMe";
        case DISK_TYPE_SCSI: return "SCSI";
        default: return "Unknown";
    }
}

/*
    Detect all storage controllers
*/
int disk_detect_controllers(void) {
	#ifdef DEBUG
    #ifdef DEBUG
    printf("DISK: Detecting storage controllers...\n");
    #endif
	#endif
    
    int controller_count = 0; // init
    
    // Scan all PCI devices for storage controllers
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint16_t vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
                if (vendor_id == 0xFFFF) {
                    if (function == 0) break;
                    continue;
                }
                /*
					Get the identification
				*/
                uint8_t class_code = pci_config_read8(bus, device, function, PCI_CLASS_CODE);
                if (class_code != PCI_CLASS_STORAGE) continue;
                
                uint16_t device_id = pci_config_read16(bus, device, function, PCI_DEVICE_ID);
                uint8_t subclass = pci_config_read8(bus, device, function, PCI_SUBCLASS);
                uint8_t prog_if = pci_config_read8(bus, device, function, PCI_PROG_IF);
                
                // Try to identify by vendor/device ID first cuz this matters
                const controller_id_t* known = identify_controller(vendor_id, device_id);
                disk_type_t type;
                const char* name;
                
                if (known) {
                    type = known->type;
                    name = known->name;
                } else {
                    // Fall back to class/subclass identification
                    type = identify_by_class(class_code, subclass, prog_if);
                    name = "Generic Controller"; // defualt to generic
                }
                
                if (type != DISK_TYPE_UNKNOWN) {
                    #ifdef DEBUG
                    printf("DISK: Found %s at %02x:%02x.%x (%04x:%04x) - %s\n", get_controller_type_name(type), bus, device, function, vendor_id, device_id, name);
                    #endif
                    // Read BARs for I/O addresses (could be just I/O or Memory mapped I/O)
                    uint32_t bar0 = pci_config_read(bus, device, function, PCI_BAR0);
                    uint32_t bar1 = pci_config_read(bus, device, function, PCI_BAR1);
                    uint32_t bar2 = pci_config_read(bus, device, function, PCI_BAR2);
                    uint32_t bar3 = pci_config_read(bus, device, function, PCI_BAR3);
                    uint32_t bar4 = pci_config_read(bus, device, function, PCI_BAR4);
                    uint32_t bar5 = pci_config_read(bus, device, function, PCI_BAR5);
                    #ifdef DEBUG
                    printf("DISK:   BARs: %08x %08x %08x %08x %08x %08x\n",
                           bar0, bar1, bar2, bar3, bar4, bar5);
                    #endif
                    // Initialize appropriate driver(some are just stubs)
                    switch (type) {
                        case DISK_TYPE_IDE_ATA:
							#ifdef DEBUG
                            #ifdef DEBUG
                            printf("DISK:   Initializing IDE/ATA driver\n");
                            #endif
							#endif
                            break;
                        case DISK_TYPE_AHCI: /*They the same thing*/
                        case DISK_TYPE_SATA:
							#ifdef DEBUG
                            #ifdef DEBUG
                            printf("DISK:   Initializing AHCI/SATA driver\n");
                            #endif
							#endif
                            break; // stub
                        case DISK_TYPE_NVME:
                            #ifdef DEBUG
                            printf("DISK:   Initializing NVMe driver\n");
                            #endif
                            break; // stub
                        case DISK_TYPE_SCSI:
                            #ifdef DEBUG
                            printf("DISK:   Initializing SCSI driver\n");
                            #endif
                            break; //stub
                        default:
                            #ifdef DEBUG
                            printf("DISK:   No driver available\n");
                            #endif
                            break;// stub
                    }
                    
                    controller_count++;
                }
                
                // Check if multifunction device (probably?)
                if (function == 0) {
                    uint8_t header_type = pci_config_read8(bus, device, function, PCI_HEADER_TYPE);
                    if (!(header_type & 0x80)) break;
                }
            }
        }
    }
    
    #ifdef DEBUG
    printf("DISK: Found %d storage controllers\n", controller_count);
    #endif
    return controller_count;
}

/*
    Initialize disk detection system
*/
void disk_detection_init(void) {
    #ifdef DEBUG
    printf("DISK: Initializing disk detection system\n");
    #endif
    
    /*
		gett the storage controllers
	*/
    int controllers = disk_detect_controllers();
    
    if (controllers == 0) {
        #ifdef DEBUG
        printf("DISK: No storage controllers found\n");
        #endif
        return;
    }
    
    #ifdef DEBUG
    printf("DISK: Detection complete\n");
    #endif
}