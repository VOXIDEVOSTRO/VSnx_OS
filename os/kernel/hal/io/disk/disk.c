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
    #ifdef DEBUG
    printf("DISK: Initializing disk subsystem\n");
    #endif
    
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
    #ifdef DEBUG
    printf("DISK: Subsystem initialized\n");
    #endif
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
        #ifdef DEBUG
        printf("DISK: Maximum disk limit reached\n");
        #endif
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
    
    #ifdef DEBUG
    printf("DISK: Registered disk %d: %s (%lu MB)\n", disk_count, disk->model, (sectors * 512) / (1024 * 1024));
    #endif
    
    disk_count++;
    return disk_count - 1;
}

/*
    Detect all disks
*/
int disk_detect_all(void) {
    #ifdef DEBUG
    printf("DISK: Detecting all storage devices\n");
    #endif
    
    int total_disks = 0;
    
    // Detect IDE/ATA disks
    #ifdef DEBUG
    printf("DISK: Scanning for IDE/ATA devices\n");
    #endif
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
	#ifdef DEBUG
	printf("DISK: Scanning for AHCI/SATA devices\n");
	#endif
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
	                    #ifdef DEBUG
	                    printf("DISK: Registered AHCI disk %d\n", disk_id);
	                    #endif
	                }
	            }
	        }
	    }
	} else {
	    #ifdef DEBUG
	    printf("DISK: No AHCI controller found or initialization failed\n");
	    #endif
	}
    
    #ifdef DEBUG
    printf("DISK: Detection complete - found %d disks\n", total_disks);
    #endif
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
        #ifdef DEBUG
        printf("DISK: Invalid disk ID %d\n", disk_id);
        #endif
		
        return -1;
    }
    
    if (lba + count > disk->sectors) {
        #ifdef DEBUG
        printf("DISK: Read beyond disk capacity\n");
        #endif
		
        return -2;
    }
    
    #ifdef DEBUG
    printf("DISK: Reading %u sectors from LBA %lu on disk %d\n", count, lba, disk_id);
    #endif
    
    // Switch to appropriate driver
    switch (disk->type) {
        case DISK_TYPE_IDE_ATA: {
            ide_device_t* ide_dev = (ide_device_t*)disk->driver_data;
            if (!ide_dev) {
                #ifdef DEBUG
                printf("DISK: No IDE device data\n");
                #endif
				
                return -3;
            }
            
            // IDE or ATA(matters on you how to say)can only read 256 sectors at once
            uint32_t sectors_read = 0;
            uint8_t* buf = (uint8_t*)buffer;
            
            while (sectors_read < count) {
                uint32_t sectors_to_read = (count - sectors_read > 256) ? 256 : (count - sectors_read);
                
                int result = ide_read_sectors(ide_dev->channel * 2 + ide_dev->drive, 
                                            lba + sectors_read, 
                                            sectors_to_read, 
                                            buf + (sectors_read * 512));
                
                if (result != 0) {
                    #ifdef DEBUG
                    printf("DISK: IDE read error %d\n", result);
                    #endif
					
                    return -4;
                }
                
                sectors_read += sectors_to_read;
            }
            
            #ifdef DEBUG
            printf("DISK: Successfully read %u sectors\n", sectors_read);
            #endif
			
            return 0;
        }
        
        case DISK_TYPE_IDE_ATAPI:
            #ifdef DEBUG
            printf("DISK: ATAPI read not implemented\n");
            #endif
			
            return -5;
            
        case DISK_TYPE_AHCI:
		case DISK_TYPE_SATA: {
		    int result = ahci_read(disk, lba, count, buffer);
		    if (result != 0) {
		        #ifdef DEBUG
		        printf("DISK: AHCI read error %d\n", result);
		        #endif
				
		        return -6;
		    }
		    #ifdef DEBUG
		    printf("DISK: Successfully read %u sectors via AHCI\n", count);
		    #endif
			
		    return 0;
		}

        case DISK_TYPE_NVME:
            #ifdef DEBUG
            printf("DISK: NVMe read not implemented\n");
            #endif
			
            return -7;
            
        case DISK_TYPE_SCSI:
            #ifdef DEBUG
            printf("DISK: SCSI read not implemented\n");
            #endif
			
            return -8;
            
        default:
            #ifdef DEBUG
            printf("DISK: Unknown disk type %d\n", disk->type);
            #endif
			
            return -9;
    }
}

/*
    Write sectors to disk
*/
int disk_write(uint8_t disk_id, uint64_t lba, uint32_t count, const void* buffer) {
	
    disk_t* disk = disk_get(disk_id);
    if (!disk) {
        #ifdef DEBUG
        printf("DISK: Invalid disk ID %d\n", disk_id);
        #endif
        return -1;
    }
    
    if (lba + count > disk->sectors) {
        #ifdef DEBUG
        printf("DISK: Write beyond disk capacity\n");
        #endif
        return -2;
    }
    
    #ifdef DEBUG
    printf("DISK: Writing %u sectors to LBA %lu on disk %d\n", count, lba, disk_id);
    #endif
    
    // Switch to appropriate driver
    switch (disk->type) {
        case DISK_TYPE_IDE_ATA: {
            ide_device_t* ide_dev = (ide_device_t*)disk->driver_data;
            if (!ide_dev) {
                #ifdef DEBUG
                printf("DISK: No IDE device data\n");
                #endif
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
                    #ifdef DEBUG
                    printf("DISK: IDE write error %d\n", result);
                    #endif
                    return -4;
					
                }
                
                sectors_written += sectors_to_write;
            }
            
            #ifdef DEBUG
            printf("DISK: Successfully wrote %u sectors\n", sectors_written);
            #endif
            return 0;
        }
        
        case DISK_TYPE_IDE_ATAPI:
            #ifdef DEBUG
            printf("DISK: ATAPI write not supported\n");
            #endif
			
            return -5;
            
        case DISK_TYPE_AHCI:
		case DISK_TYPE_SATA: {
		    int result = ahci_write(disk, lba, count, buffer);
		    if (result != 0) {
		        #ifdef DEBUG
		        printf("DISK: AHCI write error %d\n", result);
		        #endif
				
		        return -6;
		    }
		    #ifdef DEBUG
		    printf("DISK: Successfully wrote %u sectors via AHCI\n", count);
		    #endif
			
		    return 0;
		}
        case DISK_TYPE_NVME:
            #ifdef DEBUG
            printf("DISK: NVMe write not implemented\n");
            #endif
			
            return -7;
            
        case DISK_TYPE_SCSI:
            #ifdef DEBUG
            printf("DISK: SCSI write not implemented\n");
            #endif
			
            return -8;
            
        default:
            #ifdef DEBUG
            printf("DISK: Unknown disk type %d\n", disk->type);
            #endif
			
            return -9;
    }
}	

/*
    Print disk information
*/
void disk_print_info(void) {
    #ifdef DEBUG
    printf("=== DISK INFORMATION ===\n");
    #endif
    #ifdef DEBUG
    printf("Total disks: %d\n", disk_count);
    #endif
    
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
        
		/*
			Some disk data
		*/
        #ifdef DEBUG
        printf("Disk %d: %s\n", i, type_name);
        #endif
        #ifdef DEBUG
        printf("  Model: %s\n", disk->model);
        #endif
        #ifdef DEBUG
        printf("  Serial: %s\n", disk->serial);
        #endif
		#ifdef DEBUG
        printf("  Capacity: %lu MB (%lu sectors)\n", (disk->sectors * disk->sector_size) / (1024 * 1024), disk->sectors);
        #endif
		#ifdef DEBUG
        printf("  Sector Size: %u bytes\n", disk->sector_size);
        #endif
        #ifdef DEBUG
        printf("  PCI Location: %02x:%02x.%x\n", disk->bus, disk->device, disk->function);
        #endif
    }
    
    #ifdef DEBUG
    printf("========================\n");
    #endif
}

/*
    Test disk I/O
*/
int disk_test_io(uint8_t disk_id) {
    #ifdef DEBUG
    printf("DISK: Testing I/O on disk %d\n", disk_id);
    #endif
    
    disk_t* disk = disk_get(disk_id);
    if (!disk) {
        #ifdef DEBUG
        printf("DISK: Invalid disk for testing\n");
        #endif
        return -1;
    }
    
    // Allocate test buffer
    uint8_t* test_buffer = (uint8_t*)kmalloc(512);
    if (!test_buffer) {
        #ifdef DEBUG
        printf("DISK: Failed to allocate test buffer\n");
        #endif
        return -2;
    }
    
    // Read first sector
    #ifdef DEBUG
    printf("DISK: Reading sector 0...\n");
    #endif
    int result = disk_read(disk_id, 0, 1, test_buffer);
    if (result != 0) {
        #ifdef DEBUG
        printf("DISK: Read test failed with error %d\n", result);
        #endif
        kfree(test_buffer);
        return -3;
    }
    
    // Print first 16 bytes
    #ifdef DEBUG
    printf("DISK: First 16 bytes of sector 0:\n");
    #endif
    #ifdef DEBUG
    printf("DISK: ");
    #endif
    for (int i = 0; i < 16; i++) {
        #ifdef DEBUG
        printf("%02x ", test_buffer[i]);
        #endif
    }
    #ifdef DEBUG
    printf("\n");
    #endif
    
    kfree(test_buffer);
    #ifdef DEBUG
    printf("DISK: I/O test completed successfully\n");
    #endif
    return 0;
}
