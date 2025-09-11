/*
	HEADER FILES
*/
#include "ideata.h"
/*
    Global IDE state
*/
ide_channel_t ide_channels[2];
ide_device_t ide_devices[4];
uint8_t ide_irq_invoked = 0;
uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/*
    IDE register access
*/
uint8_t ide_read(uint8_t channel, uint8_t reg) {
    uint8_t result = 0;
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].noint);
    }
    
    if (reg < 0x08) {
        result = inb(ide_channels[channel].base + reg);
    } else if (reg < 0x0C) {
        result = inb(ide_channels[channel].base + reg - 0x06);
    } else if (reg < 0x0E) {
        result = inb(ide_channels[channel].ctrl + reg - 0x0A);
    } else if (reg < 0x16) {
        result = inb(ide_channels[channel].bmide + reg - 0x0E);
    }
    
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].noint);
    }
    
    return result;
}

void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].noint);
    }
    
    if (reg < 0x08) {
        outb(ide_channels[channel].base + reg, data);
    } else if (reg < 0x0C) {
        outb(ide_channels[channel].base + reg - 0x06, data);
    } else if (reg < 0x0E) {
        outb(ide_channels[channel].ctrl + reg - 0x0A, data);
    } else if (reg < 0x16) {
        outb(ide_channels[channel].bmide + reg - 0x0E, data);
    }
    
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].noint);
    }
}

void ide_read_buffer(uint8_t channel, uint8_t reg, uint32_t* buffer, uint32_t quads) {
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].noint);
    }
    
    if (reg < 0x08) {
        for (uint32_t i = 0; i < quads; i++) {
            buffer[i] = inl(ide_channels[channel].base + reg);
        }
    } else if (reg < 0x0C) {
        for (uint32_t i = 0; i < quads; i++) {
            buffer[i] = inl(ide_channels[channel].base + reg - 0x06);
        }
    } else if (reg < 0x0E) {
        for (uint32_t i = 0; i < quads; i++) {
            buffer[i] = inl(ide_channels[channel].ctrl + reg - 0x0A);
        }
    } else if (reg < 0x16) {
        for (uint32_t i = 0; i < quads; i++) {
            buffer[i] = inl(ide_channels[channel].bmide + reg - 0x0E);
        }
    }
    
    if (reg > 0x07 && reg < 0x0C) {
        ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].noint);
    }
}

/*
    Wait for IDE interrupt
*/
void ide_wait_irq(void) {
    while (!ide_irq_invoked);
    ide_irq_invoked = 0;
}

/*
    IDE interrupt handler
*/
void ide_irq(void) {
    ide_irq_invoked = 1;
}

/*
    Poll IDE status
*/
int ide_polling(uint8_t channel, uint32_t advanced_check) {
    // Wait 400ns
    for (int i = 0; i < 4; i++) {
        ide_read(channel, ATA_REG_ALTSTATUS);
    }
    
    // Wait for BSY to clear
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    if (advanced_check) {
        uint8_t state = ide_read(channel, ATA_REG_STATUS);
        
        if (state & ATA_SR_ERR) return 2; // Error
        if (state & ATA_SR_DF) return 1;  // Device fault
        if (!(state & ATA_SR_DRQ)) return 3; // DRQ should be set
    }
    
    return 0; // No error
}

/*
    Print IDE error
*/
int ide_print_error(uint32_t drive, uint8_t err) {
    if (err == 0) return err;
    
    printf("IDE: Error on drive %u: ", drive);
    
    if (err == 1) {
        printf("Device Fault\n");
        err = 19;
    } else if (err == 2) {
        uint8_t st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
        if (st & ATA_ER_AMNF) printf("No Address Mark Found\n");
        if (st & ATA_ER_TK0NF) printf("No Media or Media Error\n");
        if (st & ATA_ER_ABRT) printf("Command Aborted\n");
        if (st & ATA_ER_MCR) printf("No Media or Media Error\n");
        if (st & ATA_ER_IDNF) printf("ID not found\n");
        if (st & ATA_ER_MC) printf("No Media or Media Error\n");
        if (st & ATA_ER_UNC) printf("Uncorrectable Data Error\n");
        if (st & ATA_ER_BBK) printf("Bad Sectors\n");
        err = 22;
    } else if (err == 3) {
        printf("Reads Nothing\n");
        err = 23;
    } else if (err == 4) {
        printf("Write Protected\n");
        err = 8;
    }
    
    return err;
}

/*
    Initialize IDE controller
*/
void ide_init(void) {
    printf("IDE: Initializing IDE/ATA controller\n");
    
    // Find IDE controller via PCI
    pci_device_t* ide_pci = pci_find_class(PCI_CLASS_STORAGE, PCI_SUBCLASS_IDE);
    if (!ide_pci) {
        printf("IDE: No IDE controller found\n");
        return;
    }
    
    // Setup channels
    ide_channels[ATA_PRIMARY].base = (ide_pci->bar[0] & 0xFFFFFFFC) + 0x1F0 * (!ide_pci->bar[0]);
    ide_channels[ATA_PRIMARY].ctrl = (ide_pci->bar[1] & 0xFFFFFFFC) + 0x3F6 * (!ide_pci->bar[1]);
    ide_channels[ATA_SECONDARY].base = (ide_pci->bar[2] & 0xFFFFFFFC) + 0x170 * (!ide_pci->bar[2]);
    ide_channels[ATA_SECONDARY].ctrl = (ide_pci->bar[3] & 0xFFFFFFFC) + 0x376 * (!ide_pci->bar[3]);
    ide_channels[ATA_PRIMARY].bmide = (ide_pci->bar[4] & 0xFFFFFFFC) + 0;
    ide_channels[ATA_SECONDARY].bmide = (ide_pci->bar[4] & 0xFFFFFFFC) + 8;
    
    // Disable IRQs
    ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
    ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);
    
    printf("IDE: Controller initialized\n");
}

/*
    Detect IDE drives
*/
int ide_detect_drives(void) {
    printf("IDE: Detecting IDE drives\n");
    
    int count = 0;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            uint8_t err = 0, type = IDE_ATA, status;
            ide_devices[count].reserved = 0;
            
            // Select drive
            ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
            
            // Wait 1ms
            for (int k = 0; k < 1000; k++);
            
            // Send IDENTIFY command
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            
            // Wait 1ms
            for (int k = 0; k < 1000; k++);
            
            // Check if drive exists
            if (ide_read(i, ATA_REG_STATUS) == 0) continue;
            
            while (1) {
                status = ide_read(i, ATA_REG_STATUS);
                if ((status & ATA_SR_ERR)) {
                    err = 1;
                    break;
                }
                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break;
            }
            
            // Check for ATAPI
            if (err != 0) {
                uint8_t cl = ide_read(i, ATA_REG_LBA1);
                uint8_t ch = ide_read(i, ATA_REG_LBA2);
                
                if (cl == 0x14 && ch == 0xEB) type = IDE_ATAPI;
                else if (cl == 0x69 && ch == 0x96) type = IDE_ATAPI;
                else continue;
                
                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                for (int k = 0; k < 1000; k++);
            }
            
            // Read identification space
            uint16_t identify[256];
            for (int k = 0; k < 256; k++) {
                identify[k] = inw(ide_channels[i].base + ATA_REG_DATA);
            }
            
            // Read device parameters
            ide_devices[count].reserved = 1;
            ide_devices[count].type = type;
            ide_devices[count].channel = i;
            ide_devices[count].drive = j;
            ide_devices[count].signature = *((uint16_t*)(identify + ATA_IDENT_DEVICETYPE));
            ide_devices[count].capabilities = *((uint16_t*)(identify + ATA_IDENT_CAPABILITIES));
            ide_devices[count].commandsets = *((uint32_t*)(identify + ATA_IDENT_COMMANDSETS));
            
            // Fix capacity calculation
			if (identify[83] & 0x0400) { // 48-bit LBA supported
			    ide_devices[count].size = ((uint64_t)identify[103] << 48) |
			                             ((uint64_t)identify[102] << 32) |
			                             ((uint64_t)identify[101] << 16) |
			                             identify[100];
			} else {
			    ide_devices[count].size = (identify[61] << 16) | identify[60];
			}
            
            // Fix model name extraction (words 27-46)
			for (int k = 0; k < 20; k++) {
			    uint16_t word = identify[27 + k];
			    ide_devices[count].model[k * 2] = (word >> 8) & 0xFF;     // High byte first
			    ide_devices[count].model[k * 2 + 1] = word & 0xFF;        // Low byte second
			}
			ide_devices[count].model[40] = '\0';
            
            printf("IDE: Found %s drive: %s (%u MB)\n",
                   (type == IDE_ATA) ? "ATA" : "ATAPI",
                   ide_devices[count].model,
                   ide_devices[count].size / 1024 / 2);
            
            count++;
        }
    }
    
    return count;
}

/*
    ATA access (read/write)
*/
int ide_ata_access(uint8_t direction, uint8_t drive, uint32_t lba, uint8_t numsects, void* buffer) {
    uint8_t lba_mode, dma, cmd;
    uint8_t lba_io[6];
    uint32_t channel = ide_devices[drive].channel;
    uint32_t slavebit = ide_devices[drive].drive;
    uint32_t bus = ide_channels[channel].base;
    uint32_t words = 256;
    uint16_t cyl, i;
    uint8_t head, sect, err;
    
    // Disable interrupts
    ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].noint = 0x02);
    
    // Select drive
    if (lba >= 0x10000000) {
        // LBA48
        lba_mode = 2;
        lba_io[0] = (lba & 0x000000FF) >> 0;
        lba_io[1] = (lba & 0x0000FF00) >> 8;
        lba_io[2] = (lba & 0x00FF0000) >> 16;
        lba_io[3] = (lba & 0xFF000000) >> 24;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = 0;
    } else if (ide_devices[drive].capabilities & 0x200) {
        // LBA28
        lba_mode = 1;
        lba_io[0] = (lba & 0x00000FF) >> 0;
        lba_io[1] = (lba & 0x000FF00) >> 8;
        lba_io[2] = (lba & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba & 0xF000000) >> 24;
    } else {
        // CHS
        lba_mode = 0;
        sect = (lba % 63) + 1;
        cyl = (lba + 1 - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head = (lba + 1 - sect) % (16 * 63) / (63);
    }
    
    // Wait for drive to be ready
    while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);
    
    // Select drive
    if (lba_mode == 0) {
        ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head);
    } else {
        ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head);
    }
    
    // Write parameters
    if (lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1, 0);
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }
    ide_write(channel, ATA_REG_SECCOUNT0, numsects);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]);
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]);
    
    // Select command
    if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;
    if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
    
    ide_write(channel, ATA_REG_COMMAND, cmd);
    
    if (direction == 0) {
        // PIO Read
        for (i = 0; i < numsects; i++) {
            err = ide_polling(channel, 1);
            if (err) return err;
            
            for (int j = 0; j < words; j++) {
                ((uint16_t*)buffer)[j + i * words] = inw(bus);
            }
        }
    } else {
        // PIO Write
        for (i = 0; i < numsects; i++) {
            ide_polling(channel, 0);
            
            for (int j = 0; j < words; j++) {
                outw(bus, ((uint16_t*)buffer)[j + i * words]);
            }
        }
        
        ide_write(channel, ATA_REG_COMMAND, (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
        ide_polling(channel, 0);
    }
    
    return 0;
}

/*
    Read sectors from IDE drive
*/
int ide_read_sectors(uint8_t drive, uint32_t lba, uint8_t numsects, void* buffer) {
    if (drive > 3 || ide_devices[drive].reserved == 0) {
        return 1; // Drive not found
    }
    
    if (ide_devices[drive].type == IDE_ATAPI) {
        return 2; // ATAPI not supported for sector read
    }
    
    return ide_ata_access(ATA_READ, drive, lba, numsects, buffer);
}

/*
    Write sectors to IDE drive
*/
int ide_write_sectors(uint8_t drive, uint32_t lba, uint8_t numsects, void* buffer) {
    if (drive > 3 || ide_devices[drive].reserved == 0) {
        return 1; // Drive not found
    }
    
    if (ide_devices[drive].type == IDE_ATAPI) {
        return 2; // ATAPI not supported for sector write
    }
    
    return ide_ata_access(ATA_WRITE, drive, lba, numsects, buffer);
}

ide_device_t* ide_get_device(uint8_t index) {
    if (index >= 4) return NULL;
    return &ide_devices[index];
}