/*
	Headerz
*/
#include "pci.h"
/*
	UNCOMMENT for output or more...
*/

//#define DEBUG	/*UNCOMMENT!*/

/* Global PCI device list so declaring here*/
pci_bus_t* pci_bus_list = NULL;
pci_device_t* pci_device_list = NULL;

#ifdef DEBUG

/* Helper functions for printing numbers */
void print_num(uint32_t num) {
    if (num == 0) {
        #ifdef DEBUG
        print("0");
        #endif
        return;
    }
    char buf[16];
    int i = 0;
    uint32_t temp = num;
    while (temp > 0) {
        buf[i++] = '0' + (temp % 10);
        temp /= 10;
    }
    while (i > 0) {
        char c[2] = {buf[--i], '\0'};
        #ifdef DEBUG
        print(c);
        #endif
    }
}

void print_hex_num(uint32_t num, int digits) {
    char buf[16];
    int i = 0;
    uint32_t temp = num;
    if (temp == 0 && digits > 0) {
        for (int j = 0; j < digits; j++) {
            buf[i++] = '0';
        }
    } else {
        while (temp > 0 || i < digits) {
            if (i >= digits && temp == 0) break;
            int digit = temp % 16;
            buf[i++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
            temp /= 16;
        }
    }
    while (i > 0) {
        char c[2] = {buf[--i], '\0'};
        #ifdef DEBUG
        print(c);
        #endif
    }
}
#endif
/*
	Some I/O ports
	(just feeling lazy and copied the code from kernel/hal)
*/
/*
	[B] IO_PORTS
*/
void outb(uint16_t port, uint8_t data) { // OUT
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}
uint8_t inb(uint16_t port) { // IN
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
/*
	[W] PORTS
*/
uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}
/*
	[L] PORTS
*/
uint32_t inl(uint16_t port) {
    uint32_t result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
void outl(uint16_t port, uint32_t data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

/*

	Configuration and stuff

*/

/* Read a 32bit value from PCI configuration space, this is 32 bits, i may need some wrapper in the kernel*/
uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC) | 0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

/* read a 16bit value from PCI configuration space */
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    if (offset & 0x02) {
        return (uint16_t)(dword >> 16);
    } else {
        return (uint16_t)dword;
    }
}

/* Read an 8bit value from PCI configuration, This may need one too */
uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    uint8_t shift = (offset & 0x03) * 8;
    return (uint8_t)(dword >> shift);
}

/* write up a 32bit value to PCI configuration space */
void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) | (function << 8) | (offset & 0xFC) | 0x80000000);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

/* write a 16bit value to PCI configuration space */
void pci_config_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    if (offset & 0x02) {
        dword = (dword & 0x0000FFFF) | ((uint32_t)value << 16);
    } else {
        dword = (dword & 0xFFFF0000) | value;
    }
    pci_config_write_dword(bus, device, function, offset & 0xFC, dword);
}

/* write an 8bit value to PCI configuration space, as well as this... */
void pci_config_write_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset & 0xFC);
    uint8_t shift = (offset & 0x03) * 8;
    uint32_t mask = 0xFF << shift;
    dword = (dword & ~mask) | ((uint32_t)value << shift);
    pci_config_write_dword(bus, device, function, offset & 0xFC, dword);
}

/*

	PCI devices
	and the function for it.

*/

/* Create a new PCI */
pci_device_t* pci_device_create(uint8_t bus, uint8_t device, uint8_t function) {
    pci_device_t* dev = (pci_device_t*)umalloc(sizeof(pci_device_t));
    if (!dev) return NULL;

    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->next = NULL;

    return dev;
}

/* Destroy a PCI device, like i did when making this entire shi...LOL */
void pci_device_destroy(pci_device_t* device) {
    if (device) {
		/*
			just simple free up...
		*/
        ufree(device);
    }
}

/* read up PCI device configuration */
void pci_device_read_config(pci_device_t* device) {
    if (!device) return;

    device->vendor_id = pci_config_read_word(device->bus, device->device, device->function, PCI_VENDOR_ID);
    device->device_id = pci_config_read_word(device->bus, device->device, device->function, PCI_DEVICE_ID);
    #ifdef DEBUG
    print("Reading config for bus ");
    #endif
    #ifdef DEBUG
    print_num(device->bus);
    #endif
    #ifdef DEBUG
    print(", device ");
    #endif
    #ifdef DEBUG
    print_num(device->device);
    #endif
    #ifdef DEBUG
    print(", function ");
    #endif
    #ifdef DEBUG
    print_num(device->function);
    #endif
    #ifdef DEBUG
    print("\n");
    #endif
    #ifdef DEBUG
    print("Vendor ID: 0x");
    #endif
    #ifdef DEBUG
    print_hex_num(device->vendor_id, 4);
    #endif
    #ifdef DEBUG
    print(", Device ID: 0x");
    #endif
    #ifdef DEBUG
    print_hex_num(device->device_id, 4);
    #endif
    #ifdef DEBUG
    print("\n");
    #endif
    device->revision_id = pci_config_read_byte(device->bus, device->device, device->function, PCI_REVISION_ID);
    device->prog_if = pci_config_read_byte(device->bus, device->device, device->function, PCI_PROG_IF);
    device->subclass = pci_config_read_byte(device->bus, device->device, device->function, PCI_SUBCLASS);
    device->class_code = pci_config_read_byte(device->bus, device->device, device->function, PCI_CLASS);
    device->header_type = pci_config_read_byte(device->bus, device->device, device->function, PCI_HEADER_TYPE);

    /* spit BARs */
    for (int i = 0; i < 6; i++) {
        device->bar[i] = pci_config_read_dword(device->bus, device->device, device->function, PCI_BAR0 + (i * 4));
    }

    device->expansion_rom_base = pci_config_read_dword(device->bus, device->device, device->function, PCI_EXPANSION_ROM_BASE);
    device->interrupt_line = pci_config_read_byte(device->bus, device->device, device->function, PCI_INTERRUPT_LINE);
    device->interrupt_pin = pci_config_read_byte(device->bus, device->device, device->function, PCI_INTERRUPT_PIN);
    device->subsystem_vendor_id = pci_config_read_word(device->bus, device->device, device->function, PCI_SUBSYSTEM_VENDOR_ID);
    device->subsystem_id = pci_config_read_word(device->bus, device->device, device->function, PCI_SUBSYSTEM_ID);
}

/*

	(school)BUS management, Always....

*/

/* make up a new PCI bus */
pci_bus_t* pci_bus_create(uint8_t bus_number) {
    pci_bus_t* bus = (pci_bus_t*)umalloc(sizeof(pci_bus_t));
    if (!bus) return NULL;

    bus->bus_number = bus_number;
    bus->devices = NULL;
    bus->next = NULL;

    return bus;
}

/* Destroy a PCI (shc...i shouldn't say it)bus */
void pci_bus_destroy(pci_bus_t* bus) {
    if (!bus) return;
    pci_device_t* current = bus->devices;
    while (current) {
        pci_device_t* next = current->next;
        pci_device_destroy(current);
        current = next;
    }

    ufree(bus);
}

/* Scan up devices on the bus */
void pci_bus_scan_devices(pci_bus_t* bus) {
    if (!bus) return;

    #ifdef DEBUG
    print("Scanning bus ");
    #endif
    #ifdef DEBUG
    print_num(bus->bus_number);
    #endif
    #ifdef DEBUG
    print("\n");
    #endif

    for (uint8_t device = 0; device < 32; device++) {
        /* Check if device exists, my sanity doesnt... */
        uint16_t vendor_id = pci_config_read_word(bus->bus_number, device, 0, PCI_VENDOR_ID);
        #ifdef DEBUG
        print("Checking device ");
        #endif
        #ifdef DEBUG
        print_num(device);
        #endif
        #ifdef DEBUG
        print(", vendor_id: 0x");
        #endif
        #ifdef DEBUG
        print_hex_num(vendor_id, 4);
        #endif
        #ifdef DEBUG
        print("\n");
        #endif
        if (vendor_id == 0xFFFF) continue; /* No device */

        /* Check header type for multi-function support, i may have to robustify this */
        uint8_t header_type = pci_config_read_byte(bus->bus_number, device, 0, PCI_HEADER_TYPE);
        uint8_t max_functions = (header_type & PCI_MULTIFUNCTION) ? 8 : 1;

        for (uint8_t function = 0; function < max_functions; function++) {
            vendor_id = pci_config_read_word(bus->bus_number, device, function, PCI_VENDOR_ID);
            if (vendor_id == 0xFFFF) continue; /* No function */

            /* create device */
            pci_device_t* pci_dev = pci_device_create(bus->bus_number, device, function);
            if (!pci_dev) continue;

            /* Read config */
            pci_device_read_config(pci_dev);

            /* Add to bus */
            pci_dev->next = bus->devices;
            bus->devices = pci_dev;

            /* Add to list */
            pci_dev->next = pci_device_list;
            pci_device_list = pci_dev;
			/* dump up */
            #ifdef DEBUG
            print("PCI Device: Bus ");
            #endif
            #ifdef DEBUG
            print_num(bus->bus_number);
            #endif
            #ifdef DEBUG
            print(", Device ");
            #endif
            #ifdef DEBUG
            print_num(device);
            #endif
            #ifdef DEBUG
            print(", Function ");
            #endif
            #ifdef DEBUG
            print_num(function);
            #endif
            #ifdef DEBUG
            print(" - Vendor: 0x");
            #endif
            #ifdef DEBUG
            print_hex_num(pci_dev->vendor_id, 4);
            #endif
            #ifdef DEBUG
            print(", Device: 0x");
            #endif
            #ifdef DEBUG
            print_hex_num(pci_dev->device_id, 4);
            #endif
            #ifdef DEBUG
            print(", Class: 0x");
            #endif
            #ifdef DEBUG
            print_hex_num(pci_dev->class_code, 2);
            #endif
            #ifdef DEBUG
            print("\n");
            #endif
        }
    }
}

/*

	Enumeration (spelled it wrong???)

*/

/* Enumerate all PCI devices */
void pci_enumerate(void) {
    #ifdef DEBUG
    print("Starting PCI enumeration...\n");
    #endif

    /* Start with 0 */
    pci_bus_t* root_bus = pci_bus_create(0);
    if (!root_bus) {
        #ifdef DEBUG
        print("Failed to create root PCI bus\n");
        #endif
        return;
    }

    /* add up to bus list */
    root_bus->next = pci_bus_list;
    pci_bus_list = root_bus;

    /* scan devices on root bus */
    pci_bus_scan_devices(root_bus);

	/*
		Also i have to add support for multis...
	*/

    #ifdef DEBUG
    print("PCI enumeration completed\n");
    #endif
}

/* lookup a pci device by vendor and ID */
pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    #ifdef DEBUG
    print("Searching for device with Vendor: 0x");
    #endif
    #ifdef DEBUG
    print_hex_num(vendor_id, 4);
    #endif
    #ifdef DEBUG
    print(", Device: 0x");
    #endif
    #ifdef DEBUG
    print_hex_num(device_id, 4);
    #endif
    #ifdef DEBUG
    print("\n");
    #endif
    pci_device_t* current = pci_device_list;
    while (current) {
        #ifdef DEBUG
        print("Checking device Vendor: 0x");
        #endif
        #ifdef DEBUG
        print_hex_num(current->vendor_id, 4);
        #endif
        #ifdef DEBUG
        print(", Device: 0x");
        #endif
        #ifdef DEBUG
        print_hex_num(current->device_id, 4);
        #endif
        #ifdef DEBUG
        print("\n");
        #endif
        if (current->vendor_id == vendor_id && current->device_id == device_id) {
            #ifdef DEBUG
            print("Device found!\n");
            #endif
            return current;
        }
        current = current->next;
    }
    #ifdef DEBUG
    print("Device not found\n");
    #endif
    return NULL;
}

/* find a PCI device by class and subclass */
pci_device_t* pci_find_device_by_class(uint8_t class_code, uint8_t subclass) {
    pci_device_t* current = pci_device_list;
    while (current) {
        if (current->class_code == class_code && current->subclass == subclass) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* get the list */
pci_device_t* pci_get_devices(void) {
    return pci_device_list;
}

/*
	SPIT UP BARS!
*/

/* Get the addr of bars */
uint64_t pci_get_bar_address(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index > 5) return 0;

    uint32_t bar = device->bar[bar_index];
    uint64_t address = bar & 0xFFFFFFF0;

    /* Handle 64 BARs as well */
    if (pci_bar_is_64bit(device, bar_index) && bar_index < 5) {
        address |= (uint64_t)device->bar[bar_index + 1] << 32;
    }

    return address;
}

/* size of BAR */
uint64_t pci_get_bar_size(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index > 5) return 0;

    /* Save current BAR */
    uint32_t original_bar = device->bar[bar_index];

    /* Write all 1s to BAR to get size as this is simple*/
    pci_config_write_dword(device->bus, device->device, device->function,
                          PCI_BAR0 + (bar_index * 4), 0xFFFFFFFF);

    /* Read back */
    uint32_t size_bar = pci_config_read_dword(device->bus, device->device, device->function,
                                             PCI_BAR0 + (bar_index * 4));

    /* Restore BAR */
    pci_config_write_dword(device->bus, device->device, device->function,
                          PCI_BAR0 + (bar_index * 4), original_bar);

    /* Calculate */
    uint64_t size = ~(size_bar & 0xFFFFFFF0) + 1;

    /* Handle 64bit BARs, special handling for 'em */
    if (pci_bar_is_64bit(device, bar_index) && bar_index < 5) {
        /* Save BAR as before */
        uint32_t original_bar_high = device->bar[bar_index + 1];

        /* Write all 1s to higher BAR, can we use others too? */
        pci_config_write_dword(device->bus, device->device, device->function,
                              PCI_BAR0 + ((bar_index + 1) * 4), 0xFFFFFFFF);

        /* Read back */
        uint32_t size_bar_high = pci_config_read_dword(device->bus, device->device, device->function,
                                                      PCI_BAR0 + ((bar_index + 1) * 4));

        /* Restore original */
        pci_config_write_dword(device->bus, device->device, device->function,
                              PCI_BAR0 + ((bar_index + 1) * 4), original_bar_high);

        size = ~((uint64_t)size_bar_high << 32 | (size_bar & 0xFFFFFFF0)) + 1;
    }

    return size;
}

/* vheck if da BAR is I/O space or MMIO? */
bool pci_bar_is_io(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index > 5) return false;
    return (device->bar[bar_index] & PCI_BAR_IO_MASK) == PCI_BAR_IO;
}

/* Check OR VALIDATE if BAR is 64bit */
bool pci_bar_is_64bit(pci_device_t* device, int bar_index) {
    if (!device || bar_index < 0 || bar_index > 5) return false;
    return (device->bar[bar_index] & PCI_BAR_TYPE_MASK) == PCI_BAR_TYPE_64BIT;
}

/*

	Commands and statuses??? is that even a word???

*/

/* Master up */
void pci_enable_bus_mastering(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command |= PCI_COMMAND_MASTER;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* Master downs */
void pci_disable_bus_mastering(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_MASTER;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* toggle enable the MMIO/I/O space */
void pci_enable_io_space(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command |= PCI_COMMAND_IO;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* disable the MMIO/IO space */
void pci_disable_io_space(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_IO;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* enable if MMIO */
void pci_enable_memory_space(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command |= PCI_COMMAND_MEMORY;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* disable if MMIO */
void pci_disable_memory_space(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_MEMORY;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* interrupts toggle on */
void pci_enable_interrupts(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command &= ~PCI_COMMAND_INTX_DISABLE;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/* interrupts toggle off */
void pci_disable_interrupts(pci_device_t* device) {
    if (!device) return;
    uint16_t command = pci_config_read_word(device->bus, device->device, device->function, PCI_COMMAND);
    command |= PCI_COMMAND_INTX_DISABLE;
    pci_config_write_word(device->bus, device->device, device->function, PCI_COMMAND, command);
}

/*
	
	Main init

*/
void pci_init(void) {
    #ifdef DEBUG
    print("Initializing PCI ...\n");
    #endif

    /* Register ALL hooks */
    reg_hook("pci_config_read_dword", (hook_func_t)pci_hook_config_read_dword);
    reg_hook("pci_config_read_word", (hook_func_t)pci_hook_config_read_word);
    reg_hook("pci_config_read_byte", (hook_func_t)pci_hook_config_read_byte);
    reg_hook("pci_config_write_dword", (hook_func_t)pci_hook_config_write_dword);
    reg_hook("pci_config_write_word", (hook_func_t)pci_hook_config_write_word);
    reg_hook("pci_config_write_byte", (hook_func_t)pci_hook_config_write_byte);
    reg_hook("pci_device_create", (hook_func_t)pci_hook_device_create);
    reg_hook("pci_device_destroy", (hook_func_t)pci_hook_device_destroy);
    reg_hook("pci_device_read_config", (hook_func_t)pci_hook_device_read_config);
    reg_hook("pci_bus_create", (hook_func_t)pci_hook_bus_create);
    reg_hook("pci_bus_destroy", (hook_func_t)pci_hook_bus_destroy);
    reg_hook("pci_bus_scan_devices", (hook_func_t)pci_hook_bus_scan_devices);
    reg_hook("pci_enumerate", (hook_func_t)pci_hook_enumerate);
    reg_hook("pci_find_device", (hook_func_t)pci_hook_find_device);
    reg_hook("pci_find_device_by_class", (hook_func_t)pci_hook_find_device_by_class);
    reg_hook("pci_get_devices", (hook_func_t)pci_hook_get_devices);
    reg_hook("pci_get_bar_address", (hook_func_t)pci_hook_get_bar_address);
    reg_hook("pci_get_bar_size", (hook_func_t)pci_hook_get_bar_size);
    reg_hook("pci_bar_is_io", (hook_func_t)pci_hook_bar_is_io);
    reg_hook("pci_bar_is_64bit", (hook_func_t)pci_hook_bar_is_64bit);
    reg_hook("pci_enable_bus_mastering", (hook_func_t)pci_hook_enable_bus_mastering);
    reg_hook("pci_disable_bus_mastering", (hook_func_t)pci_hook_disable_bus_mastering);
    reg_hook("pci_enable_io_space", (hook_func_t)pci_hook_enable_io_space);
    reg_hook("pci_disable_io_space", (hook_func_t)pci_hook_disable_io_space);
    reg_hook("pci_enable_memory_space", (hook_func_t)pci_hook_enable_memory_space);
    reg_hook("pci_disable_memory_space", (hook_func_t)pci_hook_disable_memory_space);
    reg_hook("pci_enable_interrupts", (hook_func_t)pci_hook_enable_interrupts);
    reg_hook("pci_disable_interrupts", (hook_func_t)pci_hook_disable_interrupts);
    reg_hook("pci_init", (hook_func_t)pci_hook_init);
    reg_hook("pci_shutdown", (hook_func_t)pci_hook_shutdown);

    /* Enumerate up */
    pci_enumerate();

    #ifdef DEBUG
    print("PCI initialized\n");
    #endif
}

/* Shutdown (may need a driver cleanup?) */
void pci_shutdown(void) {
    #ifdef DEBUG
    print("Shutting down PCI ...\n");
    #endif

    /* Clean up device list */
    pci_device_t* current_device = pci_device_list;
    while (current_device) {
        pci_device_t* next = current_device->next;
        pci_device_destroy(current_device);
        current_device = next;
    }
    pci_device_list = NULL;

    /* Clean up list for da bus*/
    pci_bus_t* current_bus = pci_bus_list;
    while (current_bus) {
        pci_bus_t* next = current_bus->next;
        /* Devices are already freed above, in the DESTRUCTION! */
        current_bus->devices = NULL;
        pci_bus_destroy(current_bus);
        current_bus = next;
    }
    pci_bus_list = NULL;

    #ifdef DEBUG
    print("PCI shut down\n");
    #endif
}

/*

	Hook handlers

*/
int64_t pci_hook_enumerate(void* d) {
    pci_enumerate();
    return 0;
}

int64_t pci_hook_find_device(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    uint16_t vendor_id = (uint16_t)data->args[0];
    uint16_t device_id = (uint16_t)data->args[1];
    pci_device_t* device = pci_find_device(vendor_id, device_id);
    return (int64_t)device;
}

int64_t pci_hook_find_device_by_class(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    uint8_t class_code = (uint8_t)data->args[0];
    uint8_t subclass   = (uint8_t)data->args[1];
    pci_device_t* device = pci_find_device_by_class(class_code, subclass);
    return (int64_t)device;
}

int64_t pci_hook_get_devices(void* d) {
    pci_device_t* devices = pci_get_devices();
    return (int64_t)devices;
}

int64_t pci_hook_enable_bus_mastering(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_enable_bus_mastering((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_disable_bus_mastering(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_disable_bus_mastering((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_get_bar_address(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_device_t* device = (pci_device_t*)data->args[0];
    int bar_index = (int)data->args[1];
    uint64_t address = pci_get_bar_address(device, bar_index);
    return (int64_t)address;
}

int64_t pci_hook_get_bar_size(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_device_t* device = (pci_device_t*)data->args[0];
    int bar_index = (int)data->args[1];
    uint64_t size = pci_get_bar_size(device, bar_index);
    return (int64_t)size;
}

int64_t pci_hook_config_read_dword(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    return (int64_t)pci_config_read_dword(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3]
    );
}

int64_t pci_hook_config_read_word(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    return (int64_t)pci_config_read_word(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3]
    );
}

int64_t pci_hook_config_read_byte(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    return (int64_t)pci_config_read_byte(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3]
    );
}

int64_t pci_hook_config_write_dword(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_config_write_dword(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3],
        (uint32_t)data->args[4]
    );
    return 0;
}

int64_t pci_hook_config_write_word(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_config_write_word(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3],
        (uint16_t)data->args[4]
    );
    return 0;
}

int64_t pci_hook_config_write_byte(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_config_write_byte(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2],
        (uint8_t)data->args[3],
        (uint8_t)data->args[4]
    );
    return 0;
}

int64_t pci_hook_device_create(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_device_t* dev = pci_device_create(
        (uint8_t)data->args[0],
        (uint8_t)data->args[1],
        (uint8_t)data->args[2]
    );
    return (int64_t)dev;
}

int64_t pci_hook_device_destroy(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_device_destroy((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_device_read_config(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_device_read_config((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_bus_create(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_bus_t* bus = pci_bus_create((uint8_t)data->args[0]);
    return (int64_t)bus;
}

int64_t pci_hook_bus_destroy(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_bus_destroy((pci_bus_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_bus_scan_devices(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_bus_scan_devices((pci_bus_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_bar_is_io(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    bool result = pci_bar_is_io((pci_device_t*)data->args[0], (int)data->args[1]);
    return result ? 1 : 0;
}

int64_t pci_hook_bar_is_64bit(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    bool result = pci_bar_is_64bit((pci_device_t*)data->args[0], (int)data->args[1]);
    return result ? 1 : 0;
}

int64_t pci_hook_enable_io_space(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_enable_io_space((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_disable_io_space(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_disable_io_space((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_enable_memory_space(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_enable_memory_space((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_disable_memory_space(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_disable_memory_space((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_enable_interrupts(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_enable_interrupts((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_disable_interrupts(void* d) {
    struct hook_data* data = (struct hook_data*)d;
    pci_disable_interrupts((pci_device_t*)data->args[0]);
    return 0;
}

int64_t pci_hook_init(void* d) {
    pci_init();
    return 0;
}

int64_t pci_hook_shutdown(void* d) {
    pci_shutdown();
    return 0;
}
/*

	Entry point of the driver
	this is standard name for now

*/
void _start(void)
{
	/*
		Init up the pci
	*/
	pci_init();
	/*
		block up so we save up
		Also simple sleep dealy. because we need to first make the system stable before blocking
		to avoid choas such as partial init
	*/
	t_block(/*Block up*/);
	#ifdef DEBUG
	/*
		If we reach here...
	*/
	print("BLOCK IS SOOOOO STUID from PCI");
	#endif
}
