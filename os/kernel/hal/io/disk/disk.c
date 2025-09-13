/*
	HEADERS!!!!!!!!!!!!!!!!!!!!!!!
*/
#include "disk.h"
#include "ideata.h"
#include "ahci.h"
#include "nvme.h"
#include "scsi.h"
/*
	THIS will mostly hold HAL driver wrappers to provide a cenetralized control
*/
/*
    Global disk management
*/
disk_t disks[MAX_DISKS];
int disk_count = 0;
/*
    Initialize disk subsystem
*/
void disk_init(void) {
    printf("DISK: Initializing disk subsystem\n");
    
    /*
		Clear disk array
	*/
    for (int i = 0; i < MAX_DISKS; i++) {
        disks[i].id = 0xFF;
        disks[i].type = DISK_TYPE_UNKNOWN;
        disks[i].sectors = 0;
        disks[i].sector_size = 512;
        disks[i].driver_data = NULL;
    }
    
    disk_count = 0;
    printf("DISK: Subsystem initialized\n");
}

/*
    Register a disk
*/
static int disk_register(disk_type_t type, uint64_t sectors, const char* model, 
                        const char* serial, uint8_t bus, uint8_t device, 
                        uint8_t function, void* driver_data) {
	/*
		within limits?
	*/
    if (disk_count >= MAX_DISKS) {
        printf("DISK: Maximum disk limit reached\n");
        return -1;
    }
    
    disk_t* disk = &disks[disk_count];
    disk->id = disk_count;
    disk->type = type;
    disk->sectors = sectors;
    disk->sector_size = 512;
    disk->bus = bus;
    disk->device = device;
    disk->function = function;
    disk->driver_data = driver_data;
    
    // Copy model name
    int i;
    for (i = 0; i < 40 && model[i]; i++) {
        disk->model[i] = model[i];
    }
    disk->model[i] = '\0';
    
    // Copy serial number
    for (i = 0; i < 20 && serial[i]; i++) {
        disk->serial[i] = serial[i];
    }
    disk->serial[i] = '\0';
    
    printf("DISK: Registered disk %d: %s (%lu MB)\n",  // DEBUG print it
           disk_count, disk->model, (sectors * 512) / (1024 * 1024));
    
    disk_count++;
    return disk_count - 1;
}

/*
    Detect all disks
*/
int disk_detect_all(void) {
    printf("DISK: Detecting all storage devices\n");
    
    int total_disks = 0;
    
    // Detect IDE/ATA disks
    printf("DISK: Scanning for IDE/ATA devices\n");
    ide_init();
    int ide_drives = ide_detect_drives();
    
    // Register IDE drives
    for (int i = 0; i < ide_drives && i < 4; i++) {
        ide_device_t* ide_dev = ide_get_device(i);
        if (ide_dev && ide_dev->reserved) {
            int disk_id = disk_register(
                (ide_dev->type == IDE_ATA) ? DISK_TYPE_IDE_ATA : DISK_TYPE_IDE_ATAPI,
                ide_dev->size,
                (const char*)ide_dev->model,
                "IDE_SERIAL", // IDE doesn't easily provide serial
                0, 1, 1, // PCI location for IDE controller
                ide_dev
            );
            
            if (disk_id >= 0) {
                total_disks++;
            }
        }
    }

	// Detect AHCI/SATA disks
	printf("DISK: Scanning for AHCI/SATA devices\n");
	if (ahci_init() == 0) {
	    // AHCI controller initialized successfully
	    for (int i = 0; i < ahci_ctrl.port_count; i++) {
	        ahci_port_t* port = &ahci_ctrl.ports[i];
	        if (port->base != 0) {
	            // Create a temporary disk structure for identification
	            disk_t temp_disk;
	            temp_disk.driver_data = port;
			
	            // Try to identify the device
	            if (ahci_identify(&temp_disk) == 0) {
	                int disk_id = disk_register(
	                    DISK_TYPE_AHCI,
	                    temp_disk.sectors,
	                    temp_disk.model,
	                    "AHCI_SERIAL", // AHCI serial extraction needs more work
	                    0, 0, 0, // PCI location from AHCI controller
	                    port
	                );
				
	                if (disk_id >= 0) {
	                    total_disks++;
	                    printf("DISK: Registered AHCI disk %d\n", disk_id);
	                }
	            }
	        }
	    }
	} else {
	    printf("DISK: No AHCI controller found or initialization failed\n");
	}
    
    printf("DISK: Detection complete - found %d disks\n", total_disks);
    return total_disks;
}

/*
    Get disk by ID
*/
disk_t* disk_get(uint8_t id) {
    if (id >= disk_count || disks[id].type == DISK_TYPE_UNKNOWN) {
        return NULL;
    }
    return &disks[id];
}

/*
    Read sectors from disk
*/
int disk_read(uint8_t disk_id, uint64_t lba, uint32_t count, void* buffer) {
    disk_t* disk = disk_get(disk_id);
    if (!disk) {
        printf("DISK: Invalid disk ID %d\n", disk_id);
        return -1;
    }
    
    if (lba + count > disk->sectors) {
        printf("DISK: Read beyond disk capacity\n");
        return -2;
    }
    
    printf("DISK: Reading %u sectors from LBA %lu on disk %d\n", count, lba, disk_id);
    
    // Switch to appropriate driver
    switch (disk->type) {
        case DISK_TYPE_IDE_ATA: {
            ide_device_t* ide_dev = (ide_device_t*)disk->driver_data;
            if (!ide_dev) {
                printf("DISK: No IDE device data\n");
                return -3;
            }
            
            // IDE can only read 256 sectors at once
            uint32_t sectors_read = 0;
            uint8_t* buf = (uint8_t*)buffer;
            
            while (sectors_read < count) {
                uint32_t sectors_to_read = (count - sectors_read > 256) ? 256 : (count - sectors_read);
                
                int result = ide_read_sectors(ide_dev->channel * 2 + ide_dev->drive, 
                                            lba + sectors_read, 
                                            sectors_to_read, 
                                            buf + (sectors_read * 512));
                
                if (result != 0) {
                    printf("DISK: IDE read error %d\n", result);
                    return -4;
                }
                
                sectors_read += sectors_to_read;
            }
            
            printf("DISK: Successfully read %u sectors\n", sectors_read);
            return 0;
        }
        
        case DISK_TYPE_IDE_ATAPI:
            printf("DISK: ATAPI read not implemented\n");
            return -5;
            
        case DISK_TYPE_AHCI:
		case DISK_TYPE_SATA: {
		    int result = ahci_read(disk, lba, count, buffer);
		    if (result != 0) {
		        printf("DISK: AHCI read error %d\n", result);
		        return -6;
		    }
		    printf("DISK: Successfully read %u sectors via AHCI\n", count);
		    return 0;
		}

        case DISK_TYPE_NVME:
            printf("DISK: NVMe read not implemented\n");
            return -7;
            
        case DISK_TYPE_SCSI:
            printf("DISK: SCSI read not implemented\n");
            return -8;
            
        default:
            printf("DISK: Unknown disk type %d\n", disk->type);
            return -9;
    }
}

/*
    Write sectors to disk
*/
int disk_write(uint8_t disk_id, uint64_t lba, uint32_t count, const void* buffer) {
    disk_t* disk = disk_get(disk_id);
    if (!disk) {
        printf("DISK: Invalid disk ID %d\n", disk_id);
        return -1;
    }
    
    if (lba + count > disk->sectors) {
        printf("DISK: Write beyond disk capacity\n");
        return -2;
    }
    
    printf("DISK: Writing %u sectors to LBA %lu on disk %d\n", count, lba, disk_id);
    
    // Switch to appropriate driver
    switch (disk->type) {
        case DISK_TYPE_IDE_ATA: {
            ide_device_t* ide_dev = (ide_device_t*)disk->driver_data;
            if (!ide_dev) {
                printf("DISK: No IDE device data\n");
                return -3;
            }
            
            // IDE can only write 256 sectors at once
            uint32_t sectors_written = 0;
            const uint8_t* buf = (const uint8_t*)buffer;
            
            while (sectors_written < count) {
                uint32_t sectors_to_write = (count - sectors_written > 256) ? 256 : (count - sectors_written);
                
                int result = ide_write_sectors(ide_dev->channel * 2 + ide_dev->drive, 
                                             lba + sectors_written, 
                                             sectors_to_write, 
                                             (void*)(buf + (sectors_written * 512)));
                
                if (result != 0) {
                    printf("DISK: IDE write error %d\n", result);
                    return -4;
                }
                
                sectors_written += sectors_to_write;
            }
            
            printf("DISK: Successfully wrote %u sectors\n", sectors_written);
            return 0;
        }
        
        case DISK_TYPE_IDE_ATAPI:
            printf("DISK: ATAPI write not supported\n");
            return -5;
            
        case DISK_TYPE_AHCI:
		case DISK_TYPE_SATA: {
		    int result = ahci_write(disk, lba, count, buffer);
		    if (result != 0) {
		        printf("DISK: AHCI write error %d\n", result);
		        return -6;
		    }
		    printf("DISK: Successfully wrote %u sectors via AHCI\n", count);
		    return 0;
		}
        case DISK_TYPE_NVME:
            printf("DISK: NVMe write not implemented\n");
            return -7;
            
        case DISK_TYPE_SCSI:
            printf("DISK: SCSI write not implemented\n");
            return -8;
            
        default:
            printf("DISK: Unknown disk type %d\n", disk->type);
            return -9;
    }
}

/*
    Print disk information
*/
void disk_print_info(void) {
    printf("=== DISK INFORMATION ===\n");
    printf("Total disks: %d\n", disk_count);
    
    for (int i = 0; i < disk_count; i++) {
        disk_t* disk = &disks[i];
        const char* type_name;
        
        switch (disk->type) {
            case DISK_TYPE_IDE_ATA: type_name = "IDE/ATA"; break;
            case DISK_TYPE_IDE_ATAPI: type_name = "IDE/ATAPI"; break;
            case DISK_TYPE_SATA: type_name = "SATA"; break;
            case DISK_TYPE_AHCI: type_name = "AHCI"; break;
            case DISK_TYPE_NVME: type_name = "NVMe"; break;
            case DISK_TYPE_SCSI: type_name = "SCSI"; break;
            default: type_name = "Unknown"; break;
        }
        
        printf("Disk %d: %s\n", i, type_name);
        printf("  Model: %s\n", disk->model);
        printf("  Serial: %s\n", disk->serial);
        printf("  Capacity: %lu MB (%lu sectors)\n", 
               (disk->sectors * disk->sector_size) / (1024 * 1024), disk->sectors);
        printf("  Sector Size: %u bytes\n", disk->sector_size);
        printf("  PCI Location: %02x:%02x.%x\n", disk->bus, disk->device, disk->function);
    }
    
    printf("========================\n");
}

/*
    Test disk I/O
*/
int disk_test_io(uint8_t disk_id) {
    printf("DISK: Testing I/O on disk %d\n", disk_id);
    
    disk_t* disk = disk_get(disk_id);
    if (!disk) {
        printf("DISK: Invalid disk for testing\n");
        return -1;
    }
    
    // Allocate test buffer
    uint8_t* test_buffer = (uint8_t*)kmalloc(512);
    if (!test_buffer) {
        printf("DISK: Failed to allocate test buffer\n");
        return -2;
    }
    
    // Read first sector
    printf("DISK: Reading sector 0...\n");
    int result = disk_read(disk_id, 0, 1, test_buffer);
    if (result != 0) {
        printf("DISK: Read test failed with error %d\n", result);
        kfree(test_buffer);
        return -3;
    }
    
    // Print first 16 bytes
    printf("DISK: First 16 bytes of sector 0:\n");
    printf("DISK: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x ", test_buffer[i]);
    }
    printf("\n");
    
    kfree(test_buffer);
    printf("DISK: I/O test completed successfully\n");
    return 0;
}
