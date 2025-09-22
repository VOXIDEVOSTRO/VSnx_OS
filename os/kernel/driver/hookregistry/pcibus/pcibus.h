#ifndef PCIBUS_H
#define PCIBUS_H
/*
	Head
*/
#include "../../hook.h"
/*
	Prototypes
*/
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value);
void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value);
uint64_t pci_device_create(uint8_t bus, uint8_t device, uint8_t function);
void pci_device_destroy(uint64_t device_ptr);
void pci_device_read_config(uint64_t device_ptr);
uint64_t pci_bus_create(uint8_t bus_number);
void pci_bus_destroy(uint64_t bus_ptr);
void pci_bus_scan_devices(uint64_t bus_ptr);
void pci_enumerate(void);
uint64_t pci_find_devices(uint16_t vendor_id, uint16_t device_id);
uint64_t pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
uint64_t pci_get_devices(void);
uint64_t pci_get_bar_address(uint64_t device_ptr, int bar_index);
uint64_t pci_get_bar_size(uint64_t device_ptr, int bar_index);
uint8_t pci_bar_is_io(uint64_t device_ptr, int bar_index);
uint8_t pci_bar_is_64bit(uint64_t device_ptr, int bar_index);
void pci_enable_bus_mastering(uint64_t device_ptr);
void pci_disable_bus_mastering(uint64_t device_ptr);
void pci_enable_io_space(uint64_t device_ptr);
void pci_disable_io_space(uint64_t device_ptr);
void pci_enable_memory_space(uint64_t device_ptr);
void pci_disable_memory_space(uint64_t device_ptr);
void pci_enable_interrupts(uint64_t device_ptr);
void pci_disable_interrupts(uint64_t device_ptr);
void pci_init(void);
void pci_shutdown(void);
#endif
