#include "pci.h"
#include "../io/io_ports.h"
#include "../../utilities/utility.h"

/*
    PCI Device List
*/
#define MAX_PCI_DEVICES 256 // Shouldn't be unlimted?
static pci_device_t pci_devices[MAX_PCI_DEVICES];
static int pci_device_count = 0; // NULL

/*
    Read PCI configuration register
*/
uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

/*
    Write PCI configuration register
*/
void pci_config_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/*
    read 16-bit PCI configuration register
*/
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read(bus, device, function, offset);
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

/*
    read 8-bit PCI configuration register
*/
uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t data = pci_config_read(bus, device, function, offset);
    return (uint8_t)((data >> ((offset & 3) * 8)) & 0xFF);
}

/*
    scan PCI bus for device or devices
*/
int pci_scan_bus(void) {
    pci_device_count = 0;
    
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint16_t vendor_id = pci_config_read16(bus, device, function, PCI_VENDOR_ID);
                
                if (vendor_id == 0xFFFF) {
                    if (function == 0) break; /*Nothing*/
                    continue; /*nothing here either*/
                }
                
                if (pci_device_count >= MAX_PCI_DEVICES) {
                    #ifdef DEBUG
                    printf("PCI: Device limit reached\n");
                    #endif
                    return pci_device_count;
                }
                
                pci_device_t* dev = &pci_devices[pci_device_count++];
                dev->bus = bus;
                dev->device = device;
                dev->function = function;
                dev->vendor_id = vendor_id;
                dev->device_id = pci_config_read16(bus, device, function, PCI_DEVICE_ID);
                dev->class_code = pci_config_read8(bus, device, function, PCI_CLASS_CODE);
                dev->subclass = pci_config_read8(bus, device, function, PCI_SUBCLASS);
                dev->prog_if = pci_config_read8(bus, device, function, PCI_PROG_IF);
                
                // Read BARs OR SPIT BARS!!!!!!!
                for (int i = 0; i < 6; i++) {
                    dev->bar[i] = pci_config_read(bus, device, function, PCI_BAR0 + (i * 4));
                }
                
                // Check if multifunction device
                if (function == 0) {
                    uint8_t header_type = pci_config_read8(bus, device, function, PCI_HEADER_TYPE);
                    if (!(header_type & 0x80)) break; // Single function device (will handle multi ones too)
                }
            }
        }
    }
    
    return pci_device_count;
}

/*
    Find PCI device by vendor/device ID
*/
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id && pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return NULL;
}

/*
    Find PCI device by class/subclass
*/
pci_device_t* pci_find_class(uint8_t class_code, uint8_t subclass) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].class_code == class_code && pci_devices[i].subclass == subclass) {
            return &pci_devices[i];
        }
    }
    return NULL;
}

/*
    Initialize PCI
*/
void pci_init(void) {
    #ifdef DEBUG
    printf("PCI: Initializing PCI subsystem\n");
    #endif
    
    int device_count = pci_scan_bus(); // thats all we need
    #ifdef DEBUG
    printf("PCI: Found %d devices\n", device_count);
    #endif
    
    #ifdef DEBUG
    printf("PCI: Initialization complete\n");
    #endif
}

/*
    Print all PCI devices
	debugging only. slightly useful
*/
void pci_print_devices(void) {
    #ifdef DEBUG
    printf("=== PCI Devices ===\n");
    #endif
    for (int i = 0; i < pci_device_count; i++) {
        pci_device_t* dev = &pci_devices[i];
        #ifdef DEBUG
        printf("PCI %02x:%02x.%x - %04x:%04x Class %02x:%02x\n", dev->bus, dev->device, dev->function, dev->vendor_id, dev->device_id, dev->class_code, dev->subclass);
        #endif
    }
}
