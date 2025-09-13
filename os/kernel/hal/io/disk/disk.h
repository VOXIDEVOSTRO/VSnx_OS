#ifndef DISK_H
#define DISK_H
/*
	header files
*/
#include <stdint.h>
#include "../../pci/pci.h"
/*
    Disk Types
*/
typedef enum {
    DISK_TYPE_UNKNOWN = 0,
    DISK_TYPE_IDE_ATA,
    DISK_TYPE_IDE_ATAPI,
    DISK_TYPE_SATA,
    DISK_TYPE_AHCI,
    DISK_TYPE_NVME,
    DISK_TYPE_SCSI
} disk_type_t;
/*
    Known Storage Controller Vendor/Device IDs
*/
typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    disk_type_t type;
    const char* name;
} controller_id_t;
/*
    Disk Structure
*/
typedef struct {
    uint8_t id;
    disk_type_t type;
    uint64_t sectors;
    uint16_t sector_size;
    char model[41];
    char serial[21];
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    void* driver_data;
} disk_t;

#define MAX_DISKS 16

/*
    Disk Operations
	DO LI
*/
typedef struct {
    int (*read)(disk_t* disk, uint64_t lba, uint32_t count, void* buffer);
    int (*write)(disk_t* disk, uint64_t lba, uint32_t count, const void* buffer);
    int (*identify)(disk_t* disk);
} disk_ops_t;
/*
	Globals
*/
extern disk_t disks[MAX_DISKS];
extern int disk_count;

/*
    Disk Management Functions
*/
void disk_init(void);
int disk_detect_all(void);
disk_t* disk_get(uint8_t id);

/*
    Disk I/O Functions
*/
int disk_read(uint8_t disk_id, uint64_t lba, uint32_t count, void* buffer);
int disk_write(uint8_t disk_id, uint64_t lba, uint32_t count, const void* buffer);

/*
    Disk Information Functions
*/
void disk_print_info(void);
int disk_test_io(uint8_t disk_id);

/*
    Disk Registration Functions (internal)
*/
static int disk_register(disk_type_t type, uint64_t sectors, const char* model, 
                        const char* serial, uint8_t bus, uint8_t device, 
                        uint8_t function, void* driver_data);
#endif