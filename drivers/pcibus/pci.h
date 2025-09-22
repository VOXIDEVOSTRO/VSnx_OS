#ifndef PCI_H
#define PCI_H
/*
	header files
	as always..
*/
#include <stdint.h>
#include <stdbool.h>
/*
	Some hook stuff
*/
#ifndef INUSE
#include "../../os/kernel/driver/hookcall/hookreg.h"
/*
	just for the syscalls and user memory...
*/
#include "../../os/kernel/syscalls/func.h"
#endif
/*
	Registers
*/
#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_REVISION_ID         0x08
#define PCI_PROG_IF             0x09
#define PCI_SUBCLASS            0x0A
#define PCI_CLASS               0x0B
#define PCI_CACHE_LINE_SIZE     0x0C
#define PCI_LATENCY_TIMER       0x0D
#define PCI_HEADER_TYPE         0x0E
#define PCI_BIST                0x0F
#define PCI_BAR0                0x10
#define PCI_BAR1                0x14
#define PCI_BAR2                0x18
#define PCI_BAR3                0x1C
#define PCI_BAR4                0x20
#define PCI_BAR5                0x24
#define PCI_CARDBUS_CIS         0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C
#define PCI_SUBSYSTEM_ID        0x2E
#define PCI_EXPANSION_ROM_BASE  0x30
#define PCI_CAPABILITIES_PTR    0x34
#define PCI_INTERRUPT_LINE      0x3C
#define PCI_INTERRUPT_PIN       0x3D
#define PCI_MIN_GNT             0x3E
#define PCI_MAX_LAT             0x3F

/*
	header types
*/
#define PCI_HEADER_TYPE_NORMAL      0x00
#define PCI_HEADER_TYPE_BRIDGE      0x01
#define PCI_HEADER_TYPE_CARDBUS     0x02
#define PCI_MULTIFUNCTION           0x80

/*
	And the CRBs
*/
#define PCI_COMMAND_IO              0x0001
#define PCI_COMMAND_MEMORY          0x0002
#define PCI_COMMAND_MASTER          0x0004
#define PCI_COMMAND_SPECIAL         0x0008
#define PCI_COMMAND_INVALIDATE      0x0010
#define PCI_COMMAND_VGA_PALETTE     0x0020
#define PCI_COMMAND_PARITY          0x0040
#define PCI_COMMAND_WAIT            0x0080
#define PCI_COMMAND_SERR            0x0100
#define PCI_COMMAND_FAST_BACK       0x0200
#define PCI_COMMAND_INTX_DISABLE    0x0400

/*
	Also the SRBs
*/
#define PCI_STATUS_CAP_LIST         0x0010
#define PCI_STATUS_66MHZ            0x0020
#define PCI_STATUS_UDF              0x0040
#define PCI_STATUS_FAST_BACK        0x0080
#define PCI_STATUS_PARITY           0x0100
#define PCI_STATUS_DEVSEL_MASK      0x0600
#define PCI_STATUS_DEVSEL_FAST      0x0000
#define PCI_STATUS_DEVSEL_MEDIUM    0x0200
#define PCI_STATUS_DEVSEL_SLOW      0x0400
#define PCI_STATUS_SIG_TARGET_ABORT 0x0800
#define PCI_STATUS_REC_TARGET_ABORT 0x1000
#define PCI_STATUS_REC_MASTER_ABORT 0x2000
#define PCI_STATUS_SIG_SYSTEM_ERROR 0x4000
#define PCI_STATUS_DETECTED_PARITY  0x8000

/*
	Some standard classes
*/
#define PCI_CLASS_UNCLASSIFIED      0x00
#define PCI_CLASS_STORAGE           0x01
#define PCI_CLASS_NETWORK           0x02
#define PCI_CLASS_DISPLAY           0x03
#define PCI_CLASS_MULTIMEDIA        0x04
#define PCI_CLASS_MEMORY            0x05
#define PCI_CLASS_BRIDGE            0x06
#define PCI_CLASS_COMMUNICATION     0x07
#define PCI_CLASS_SYSTEM            0x08
#define PCI_CLASS_INPUT             0x09
#define PCI_CLASS_DOCKING           0x0A
#define PCI_CLASS_PROCESSOR         0x0B
#define PCI_CLASS_SERIAL_BUS        0x0C
#define PCI_CLASS_WIRELESS          0x0D
#define PCI_CLASS_INTELLIGENT_IO    0x0E
#define PCI_CLASS_SATELLITE         0x0F
#define PCI_CLASS_ENCRYPTION        0x10
#define PCI_CLASS_SIGNAL_PROCESSING 0x11
#define PCI_CLASS_PROCESSING_ACCEL  0x12
#define PCI_CLASS_NON_ESSENTIAL     0x13
#define PCI_CLASS_COPROCESSOR       0x40

/*
	Subclasses
*/
#define PCI_SUBCLASS_STORAGE_SCSI       0x00
#define PCI_SUBCLASS_STORAGE_IDE        0x01
#define PCI_SUBCLASS_STORAGE_FLOPPY     0x02
#define PCI_SUBCLASS_STORAGE_IPI        0x03
#define PCI_SUBCLASS_STORAGE_RAID       0x04
#define PCI_SUBCLASS_STORAGE_ATA        0x05
#define PCI_SUBCLASS_STORAGE_SATA       0x06
#define PCI_SUBCLASS_STORAGE_SAS        0x07
#define PCI_SUBCLASS_STORAGE_NVM        0x08
#define PCI_SUBCLASS_STORAGE_UFS        0x09

/*
	unimplimneted,
	but nice to have, network classes
*/
#define PCI_SUBCLASS_NETWORK_ETHERNET   0x00
#define PCI_SUBCLASS_NETWORK_TOKEN_RING 0x01
#define PCI_SUBCLASS_NETWORK_FDDI       0x02
#define PCI_SUBCLASS_NETWORK_ATM        0x03
#define PCI_SUBCLASS_NETWORK_ISDN       0x04
#define PCI_SUBCLASS_NETWORK_WORLDFIP   0x05
#define PCI_SUBCLASS_NETWORK_PICMG      0x06
#define PCI_SUBCLASS_NETWORK_INFINIBAND 0x07
#define PCI_SUBCLASS_NETWORK_FABRIC     0x08

/*
	Subclasses for display
*/
#define PCI_SUBCLASS_DISPLAY_VGA        0x00
#define PCI_SUBCLASS_DISPLAY_XGA        0x01
#define PCI_SUBCLASS_DISPLAY_3D         0x02
#define PCI_SUBCLASS_DISPLAY_OTHER      0x80

/*
	Some serial stuff
*/
#define PCI_SUBCLASS_SERIAL_FIREWIRE    0x00
#define PCI_SUBCLASS_SERIAL_ACCESS      0x01
#define PCI_SUBCLASS_SERIAL_SSA         0x02
#define PCI_SUBCLASS_SERIAL_USB         0x03
#define PCI_SUBCLASS_SERIAL_FIBER       0x04
#define PCI_SUBCLASS_SERIAL_SMBUS       0x05
#define PCI_SUBCLASS_SERIAL_INFINIBAND  0x06
#define PCI_SUBCLASS_SERIAL_IPMI        0x07
#define PCI_SUBCLASS_SERIAL_SERCOS      0x08
#define PCI_SUBCLASS_SERIAL_CANBUS      0x09

/*
	BARS
*/
#define PCI_BAR_TYPE_MASK           0x00000001
#define PCI_BAR_TYPE_32BIT          0x00000000
#define PCI_BAR_TYPE_64BIT          0x00000001
#define PCI_BAR_IO_MASK             0x00000001
#define PCI_BAR_IO                  0x00000001
#define PCI_BAR_MEMORY_MASK         0x0000000F
#define PCI_BAR_MEMORY_32BIT        0x00000000
#define PCI_BAR_MEMORY_1MB          0x00000002
#define PCI_BAR_MEMORY_64BIT        0x00000004

/*
	Simple PCI structure
*/
typedef struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t header_type;
    uint32_t bar[6];
    uint32_t expansion_rom_base;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    struct pci_device* next;
} pci_device_t;

/*
	BUS
*/
typedef struct pci_bus {
    uint8_t bus_number;
    pci_device_t* devices;
    struct pci_bus* next;
} pci_bus_t;

/*
	Globals
*/
pci_bus_t* pci_bus_list;
pci_device_t* pci_device_list;

/*
	Prototypes
*/
#ifndef INUSE
#define INUSE
void print_num(uint32_t num);
void print_hex_num(uint32_t num, int digits);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);
uint32_t inl(uint16_t port);
void outl(uint16_t port, uint32_t data);
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);
pci_device_t *pci_device_create(uint8_t bus, uint8_t device, uint8_t function);
void pci_device_destroy(pci_device_t *device);
void pci_device_read_config(pci_device_t *device);
pci_bus_t *pci_bus_create(uint8_t bus_number);
void pci_bus_destroy(pci_bus_t *bus);
void pci_bus_scan_devices(pci_bus_t *bus);
void pci_enumerate(void);
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t *pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
pci_device_t *pci_get_devices(void);
uint64_t pci_get_bar_address(pci_device_t *device, int bar_index);
uint64_t pci_get_bar_size(pci_device_t *device, int bar_index);
_Bool pci_bar_is_io(pci_device_t *device, int bar_index);
_Bool pci_bar_is_64bit(pci_device_t *device, int bar_index);
void pci_enable_bus_mastering(pci_device_t *device);
void pci_disable_bus_mastering(pci_device_t *device);
void pci_enable_io_space(pci_device_t *device);
void pci_disable_io_space(pci_device_t *device);
void pci_enable_memory_space(pci_device_t *device);
void pci_disable_memory_space(pci_device_t *device);
void pci_enable_interrupts(pci_device_t *device);
void pci_disable_interrupts(pci_device_t *device);
void pci_init(void);
void pci_shutdown(void);
int64_t pci_hook_enumerate(void *d);
int64_t pci_hook_find_device(void *d);
int64_t pci_hook_find_device_by_class(void *d);
int64_t pci_hook_get_devices(void *d);
int64_t pci_hook_enable_bus_mastering(void *d);
int64_t pci_hook_disable_bus_mastering(void *d);
int64_t pci_hook_get_bar_address(void *d);
int64_t pci_hook_get_bar_size(void *d);
int64_t pci_hook_config_read_dword(void *d);
int64_t pci_hook_config_read_word(void *d);
int64_t pci_hook_config_read_byte(void *d);
int64_t pci_hook_config_write_dword(void *d);
int64_t pci_hook_config_write_word(void *d);
int64_t pci_hook_config_write_byte(void *d);
int64_t pci_hook_device_create(void *d);
int64_t pci_hook_device_destroy(void *d);
int64_t pci_hook_device_read_config(void *d);
int64_t pci_hook_bus_create(void *d);
int64_t pci_hook_bus_destroy(void *d);
int64_t pci_hook_bus_scan_devices(void *d);
int64_t pci_hook_bar_is_io(void *d);
int64_t pci_hook_bar_is_64bit(void *d);
int64_t pci_hook_enable_io_space(void *d);
int64_t pci_hook_disable_io_space(void *d);
int64_t pci_hook_enable_memory_space(void *d);
int64_t pci_hook_disable_memory_space(void *d);
int64_t pci_hook_enable_interrupts(void *d);
int64_t pci_hook_disable_interrupts(void *d);
int64_t pci_hook_init(void *d);
int64_t pci_hook_shutdown(void *d);
#endif
#endif
