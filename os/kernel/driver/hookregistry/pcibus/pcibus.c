/*
	Header files
*/
#include "pcibus.h"
/*
	main functions
*/
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    HOOK_CALL("pci_config_read_dword", &data);
    return (uint32_t)data.args[0];
}

uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    HOOK_CALL("pci_config_read_word", &data);
    return (uint16_t)data.args[0];
}

uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    HOOK_CALL("pci_config_read_byte", &data);
    return (uint8_t)data.args[0];
}

void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    data.args[4] = value;
    HOOK_CALL("pci_config_write_dword", &data);
}

void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    data.args[4] = value;
    HOOK_CALL("pci_config_write_word", &data);
}

void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    data.args[3] = offset;
    data.args[4] = value;
    HOOK_CALL("pci_config_write_byte", &data);
}

uint64_t pci_device_create(uint8_t bus, uint8_t device, uint8_t function) {
    struct hook_data data = {0};
    data.args[0] = bus;
    data.args[1] = device;
    data.args[2] = function;
    HOOK_CALL("pci_device_create", &data);
    return (uint64_t)data.args[0];
}

void pci_device_destroy(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_device_destroy", &data);
}

void pci_device_read_config(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_device_read_config", &data);
}

uint64_t pci_bus_create(uint8_t bus_number) {
    struct hook_data data = {0};
    data.args[0] = bus_number;
    HOOK_CALL("pci_bus_create", &data);
    return (uint64_t)data.args[0];
}

void pci_bus_destroy(uint64_t bus_ptr) {
    struct hook_data data = {0};
    data.args[0] = bus_ptr;
    HOOK_CALL("pci_bus_destroy", &data);
}

void pci_bus_scan_devices(uint64_t bus_ptr) {
    struct hook_data data = {0};
    data.args[0] = bus_ptr;
    HOOK_CALL("pci_bus_scan_devices", &data);
}

void pci_enumerate(void) {
    struct hook_data data = {0};
    HOOK_CALL("pci_enumerate", &data);
}

uint64_t pci_find_devices(uint16_t vendor_id, uint16_t device_id) {
    struct hook_data data = {0};
    data.args[0] = vendor_id;
    data.args[1] = device_id;
    HOOK_CALL("pci_find_device", &data);
    return (uint64_t)data.args[0];
}

uint64_t pci_find_device_by_class(uint8_t class_code, uint8_t subclass) {
    struct hook_data data = {0};
    data.args[0] = class_code;
    data.args[1] = subclass;
    HOOK_CALL("pci_find_device_by_class", &data);
    return (uint64_t)data.args[0];
}

uint64_t pci_get_devices(void) {
    struct hook_data data = {0};
    HOOK_CALL("pci_get_devices", &data);
    return (uint64_t)data.args[0];
}

uint64_t pci_get_bar_address(uint64_t device_ptr, int bar_index) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    data.args[1] = bar_index;
    HOOK_CALL("pci_get_bar_address", &data);
    return (uint64_t)data.args[0];
}

uint64_t pci_get_bar_size(uint64_t device_ptr, int bar_index) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    data.args[1] = bar_index;
    HOOK_CALL("pci_get_bar_size", &data);
    return (uint64_t)data.args[0];
}

uint8_t pci_bar_is_io(uint64_t device_ptr, int bar_index) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    data.args[1] = bar_index;
    HOOK_CALL("pci_bar_is_io", &data);
    return (uint8_t)data.args[0];
}

uint8_t pci_bar_is_64bit(uint64_t device_ptr, int bar_index) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    data.args[1] = bar_index;
    HOOK_CALL("pci_bar_is_64bit", &data);
    return (uint8_t)data.args[0];
}

void pci_enable_bus_mastering(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_enable_bus_mastering", &data);
}

void pci_disable_bus_mastering(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_disable_bus_mastering", &data);
}

void pci_enable_io_space(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_enable_io_space", &data);
}

void pci_disable_io_space(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_disable_io_space", &data);
}

void pci_enable_memory_space(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_enable_memory_space", &data);
}

void pci_disable_memory_space(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_disable_memory_space", &data);
}

void pci_enable_interrupts(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_enable_interrupts", &data);
}

void pci_disable_interrupts(uint64_t device_ptr) {
    struct hook_data data = {0};
    data.args[0] = device_ptr;
    HOOK_CALL("pci_disable_interrupts", &data);
}

void pci_init(void) {
    struct hook_data data = {0};
    HOOK_CALL("pci_init", &data);
}

void pci_shutdown(void) {
    struct hook_data data = {0};
    HOOK_CALL("pci_shutdown", &data);
}