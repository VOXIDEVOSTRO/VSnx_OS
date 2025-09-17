/*
    BLOCK DEVICE ADAPTER
    Connects Gristle FAT driver to HAL disk I/O
*/
#include "block.h"
#include "../hal/io/disk/disk.h"
#include "../utilities/utility.h"
/*
	BLOCK state
*/
uint8_t current_disk = 0;
int block_initialized = 0;
int last_error = 0;
int read_only = 0;

/*
    Any setup needed by the driver
*/
int block_init() {
    #ifdef DEBUG
    printf("BLOCK: Initializing block device system\n");
    #endif
    /*
		TO BE CHANGED: make it properly identify the boot DRIVE!
	*/
    // Use disk 0 by default (becuase could be the boot drive)
    disk_t* disk = disk_get(0);
    if (!disk) {
        #ifdef DEBUG
        printf("BLOCK: No disk 0 available\n");
        #endif
        last_error = -1;
        return -1;
    }
    
    if (disk->sector_size != BLOCK_SIZE) {
        #ifdef DEBUG
        printf("BLOCK: Disk sector size %d != %d\n", disk->sector_size, BLOCK_SIZE);
        #endif
        last_error = -2;
        return -2;
    }
    
    current_disk = 0;
    block_initialized = 1;
    last_error = 0;
    read_only = 0; // Assume writable for now. TO BE FIXED. PROPERLY CHECK
    
    #ifdef DEBUG
    printf("BLOCK: Block device initialized (disk %d, %d-byte blocks)\n", current_disk, BLOCK_SIZE);
    #endif
    return 0;
}

/*
    Halt the block driver
*/
int block_halt() {
    #ifdef DEBUG
    printf("BLOCK: Halting block device\n");
    #endif
    block_initialized = 0;
    last_error = 0;
    return 0;
}

/*
    Read the specified block number into memory
*/
int block_read(blockno_t block, void *buf) {
    if (!block_initialized) {
        last_error = -10;
        return -1;
    }
    
    if (!buf) {
        last_error = -11;
        return -1;
    }
    
    // Read one 512-byte sector as some legacy use them and ourt fat32 driver
    int result = disk_read(current_disk, (uint64_t)block, 1, buf);
    
    if (result != 0) {
        #ifdef DEBUG
        printf("BLOCK: Read error %d at block %u\n", result, block);
        #endif
        last_error = result;
        return -1;
    }
    
    last_error = 0;
    return 0; // Success
}

/*
    Write a block from memory to the volume
*/
int block_write(blockno_t block, void *buf) {
    if (!block_initialized) {
        last_error = -20;
        return -1;
    }
    
    if (!buf) {
        last_error = -21;
        return -1;
    }
    
    if (read_only) {
        last_error = -22;
        return -1;
    }
    
    // Write one 512-byte sector (same)
    int result = disk_write(current_disk, (uint64_t)block, 1, buf);
    
    if (result != 0) {
        #ifdef DEBUG
        printf("BLOCK: Write error %d at block %u\n", result, block);
        #endif
        last_error = result;
        return -1;
    }
    
    last_error = 0;
    return 0; // Success
}

/*
    Get the size of the volume in blocks
*/
blockno_t block_get_volume_size() {
    if (!block_initialized) {
        return 0;
    }
    
    disk_t* disk = disk_get(current_disk);
    if (!disk) {
        return 0;
    }
    
    // Return total sectors (blocks) on disk
    return (blockno_t)disk->sectors;
}

/*
    Returns the compiled value of BLOCK_SIZE
*/
int block_get_block_size() {
    return BLOCK_SIZE;
}

/*
    Find out if the volume is mounted as read only
*/
int block_get_device_read_only() {
    return read_only;
}

/*
    Get error description from the block driver layer
*/
int block_get_error() {
    return last_error;
}

/*
	This is for kernel
*/
void block_set_device(uint8_t disk_id) {
    current_disk = disk_id;
	/*
		We can change disks
	*/
    block_halt();
    block_init();
}

void block_set_read_only(int ro) {
    read_only = ro;
}
