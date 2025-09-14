/*
	Header files
*/
#include "disk.h"
#include "ahci.h"
#include "../../pci/pci.h" // PCI
#include "../../mem/k_mem/k_mem.h" // Memory mapping for MMIO
#include "../../../utilities/utility.h" // printf. duh..
/*
	Declaring globals here
*/
ahci_controller_t ahci_ctrl;
/*
	Helpers: read the ahci register
*/
static uint32_t ahci_read_reg(uint32_t base, uint32_t offset) {
    return *(volatile uint32_t*)(base + offset);
}
/*
	Healpers: write the ahci register
*/
static void ahci_write_reg(uint32_t base, uint32_t offset, uint32_t value) {
    *(volatile uint32_t*)(base + offset) = value;
}
/*
	AHCI INIT! 
*/
int ahci_init(void) {
    /*
		Find the AHCI controller
	*/
    pci_device_t* ahci_dev = pci_find_class(0x01, 0x06);
    if (!ahci_dev) {
		#ifdef DEBUG
        printf("AHCI: No AHCI controller found\n");
		#endif
        return -1;
    }

    /*
		Get the ABAR
	*/
    ahci_ctrl.abar = ahci_dev->bar[5] & 0xFFFFFFF0;
    
    if (ahci_ctrl.abar == 0) {
		#ifdef DEBUG
        printf("AHCI: Invalid ABAR address\n");
		#endif
        return -1;
    }
    #ifdef DEBUG
    printf("AHCI: Controller found at ABAR 0x%x\n", ahci_ctrl.abar);
	#endif

    /*
		Map the I/O or MMIO for ABAR
	*/
    if (paging_map_page(ahci_ctrl.abar, ahci_ctrl.abar, PAGE_PRESENT | PAGE_WRITABLE) == 0) {
		#ifdef DEBUG
        printf("AHCI: Failed to map MMIO region\n");
		#endif
        return -1;
    }

    /*
		enable AHCI
	*/
    uint32_t ghc = ahci_read_reg(ahci_ctrl.abar, AHCI_GHC_GHC);
    ghc |= (1 << 31); // AHCI Enable
    ahci_write_reg(ahci_ctrl.abar, AHCI_GHC_GHC, ghc);

    /*
		Get da ports
	*/
    ahci_ctrl.ports_impl = ahci_read_reg(ahci_ctrl.abar, AHCI_GHC_PI);
	#ifdef DEBUG
    printf("AHCI: Ports implemented: 0x%x\n", ahci_ctrl.ports_impl);
	#endif

    /*
		Init the ports
	*/
    ahci_ctrl.port_count = 0;
    for (int i = 0; i < 32; i++) {
        if (ahci_ctrl.ports_impl & (1 << i)) {
            if (ahci_port_init(&ahci_ctrl, i) == 0) {
                ahci_ctrl.port_count++;
            }
        }
    }
	#ifdef DEBUG
    printf("AHCI: Initialized %d ports\n", ahci_ctrl.port_count);
	#endif
    return 0; // Done!
}

/*
	Init the ports
*/
int ahci_port_init(ahci_controller_t* ctrl, uint8_t port_num) {
    uint32_t port_base = ctrl->abar + 0x100 + (port_num * 0x80);
    ahci_port_t* port = &ctrl->ports[port_num];
    
    port->base = port_base;
    port->port_num = port_num;

    /*
		Check if the device is present and active
	*/
    uint32_t ssts = ahci_read_reg(port_base, AHCI_PORT_SSTS);
    if ((ssts & 0xF) != 3) {
        return -1; // EROR
    }

    /*
		Stop the command engine
	*/
    uint32_t cmd = ahci_read_reg(port_base, AHCI_PORT_CMD);
    cmd &= ~(1 << 0); // Clear ST
    ahci_write_reg(port_base, AHCI_PORT_CMD, cmd);

    /*
		Wait
	*/
    int timeout = 1000;
    while ((ahci_read_reg(port_base, AHCI_PORT_CMD) & (1 << 15)) && timeout--) {
    }

    /*
		alloc memory structures
	*/
    port->cmd_list = (ahci_cmd_header_t*)kmalloc(1024);
    port->fis_base = (uint8_t*)kmalloc(256);
    port->cmd_table = (ahci_cmd_table_t*)kmalloc(256);

    if (!port->cmd_list || !port->fis_base || !port->cmd_table) {
		#ifdef DEBUG
        printf("AHCI: Failed to allocate memory for port %d\n", port_num);
		#endif
        return -1;
    }

	/*
		initialize the port
	*/
    memset(port->cmd_list, 0, 1024);
    memset(port->fis_base, 0, 256);
    memset(port->cmd_table, 0, 256);

    /*
		set command list
	*/
    ahci_write_reg(port_base, AHCI_PORT_CLB, (uint32_t)port->cmd_list);
    ahci_write_reg(port_base, AHCI_PORT_CLBU, 0);

    /*
		set the FIS
	*/
    ahci_write_reg(port_base, AHCI_PORT_FB, (uint32_t)port->fis_base);
    ahci_write_reg(port_base, AHCI_PORT_FBU, 0);

    /*
		Set up the command header
	*/
    port->cmd_list[0].cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    port->cmd_list[0].prdtl = 1;
    port->cmd_list[0].ctba = (uint32_t)port->cmd_table;
    port->cmd_list[0].ctbau = 0;

    /*
		enable FIS recive
	*/
    cmd = ahci_read_reg(port_base, AHCI_PORT_CMD);
    cmd |= (1 << 4); // FRE
    ahci_write_reg(port_base, AHCI_PORT_CMD, cmd);

    /*
		restart the command engine
	*/
    cmd |= (1 << 0); // ST
    ahci_write_reg(port_base, AHCI_PORT_CMD, cmd);

	#ifdef DEBUG
    printf("AHCI: Port %d initialized\n", port_num);
	#endif
    return 0; // DONE
}
/*
	simple execute command for AHCI
*/
static int ahci_exec_cmd(ahci_port_t* port, uint8_t cmd_slot) {
    uint32_t port_base = port->base;
    
    /*
		Issue the command (write)
	*/
    ahci_write_reg(port_base, AHCI_PORT_CI, 1 << cmd_slot);
    
    int timeout = 10000;
    while ((ahci_read_reg(port_base, AHCI_PORT_CI) & (1 << cmd_slot)) && timeout--) {
        /*
			check if ERRO
		*/
        if (ahci_read_reg(port_base, AHCI_PORT_IS) & (1 << 30)) {
			#ifdef DEBUG
            printf("AHCI: Command error\n");
			#endif
            return -1;
        }
    }
    
    if (timeout <= 0) {
		#ifdef DEBUG
        printf("AHCI: Command timeout\n");
		#endif
        return -1;
    }
    
    return 0; /*Command executed*/
}
/*
	THE main I/O for AHCI
	read, write, identify
*/
/*
	READ
*/
int ahci_read(void* disk_ptr, uint64_t lba, uint32_t count, void* buffer) {
    disk_t* disk = (disk_t*)disk_ptr;
    ahci_port_t* port = (ahci_port_t*)disk->driver_data;
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)port->cmd_table->cfis;
    memset(cmd_fis, 0, sizeof(fis_reg_h2d_t));
    
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1; // Command LOL (okay i gotta stop making so much fun)
    cmd_fis->command = 0x25; // READ DMA EXT
    
    cmd_fis->lba0 = lba & 0xFF;
    cmd_fis->lba1 = (lba >> 8) & 0xFF;
    cmd_fis->lba2 = (lba >> 16) & 0xFF;
    cmd_fis->lba3 = (lba >> 24) & 0xFF;
    cmd_fis->lba4 = (lba >> 32) & 0xFF;
    cmd_fis->lba5 = (lba >> 40) & 0xFF;
    
    cmd_fis->device = 1 << 6; // LBA mode
    cmd_fis->countl = count & 0xFF;
    cmd_fis->counth = (count >> 8) & 0xFF;
    
    port->cmd_table->prdt_entry[0].dba = (uint32_t)buffer;
    port->cmd_table->prdt_entry[0].dbau = 0;
    port->cmd_table->prdt_entry[0].dbc = (count * 512) - 1;
    port->cmd_table->prdt_entry[0].i = 0;
    
    port->cmd_list[0].w = 0; // Read
    port->cmd_list[0].prdbc = 0;
    
    return ahci_exec_cmd(port, 0);
}
/*
	WRITE
*/
int ahci_write(void* disk_ptr, uint64_t lba, uint32_t count, const void* buffer) {
    disk_t* disk = (disk_t*)disk_ptr;
    ahci_port_t* port = (ahci_port_t*)disk->driver_data;
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)port->cmd_table->cfis;
    memset(cmd_fis, 0, sizeof(fis_reg_h2d_t));
    
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1;
    cmd_fis->command = 0x35; // WRITE DMA EXT
    
    cmd_fis->lba0 = lba & 0xFF;
    cmd_fis->lba1 = (lba >> 8) & 0xFF;
    cmd_fis->lba2 = (lba >> 16) & 0xFF;
    cmd_fis->lba3 = (lba >> 24) & 0xFF;
    cmd_fis->lba4 = (lba >> 32) & 0xFF;
    cmd_fis->lba5 = (lba >> 40) & 0xFF;
    
    cmd_fis->device = 1 << 6;
    cmd_fis->countl = count & 0xFF;
    cmd_fis->counth = (count >> 8) & 0xFF;
    
    port->cmd_table->prdt_entry[0].dba = (uint32_t)buffer;
    port->cmd_table->prdt_entry[0].dbau = 0;
    port->cmd_table->prdt_entry[0].dbc = (count * 512) - 1;
    
    port->cmd_list[0].w = 1; // Write
    port->cmd_list[0].prdbc = 0;
    
    return ahci_exec_cmd(port, 0);
}
/*
	IDENTITFY
*/
int ahci_identify(void* disk_ptr) {
    disk_t* disk = (disk_t*)disk_ptr;
    ahci_port_t* port = (ahci_port_t*)disk->driver_data;
    uint16_t* identify_data = (uint16_t*)kmalloc(512);
    
    if (!identify_data) {
        return -1;
    }
    
    fis_reg_h2d_t* cmd_fis = (fis_reg_h2d_t*)port->cmd_table->cfis;
    memset(cmd_fis, 0, sizeof(fis_reg_h2d_t));
    
    cmd_fis->fis_type = FIS_TYPE_REG_H2D;
    cmd_fis->c = 1;
    cmd_fis->command = 0xEC; // IDENTIFY DEVICE
    
    // Setup PRDT
    port->cmd_table->prdt_entry[0].dba = (uint32_t)identify_data;
    port->cmd_table->prdt_entry[0].dbau = 0;
    port->cmd_table->prdt_entry[0].dbc = 511;
    
    port->cmd_list[0].w = 0;
    port->cmd_list[0].prdbc = 0;
    
    if (ahci_exec_cmd(port, 0) != 0) {
        kfree(identify_data);
        return -1;
    }
    
    disk->sectors = ((uint64_t)identify_data[103] << 48) |
                   ((uint64_t)identify_data[102] << 32) |
                   ((uint64_t)identify_data[101] << 16) |
                   identify_data[100];

    for (int i = 0; i < 20; i++) {
        disk->model[i*2] = identify_data[27+i] >> 8;
        disk->model[i*2+1] = identify_data[27+i] & 0xFF;
    }
    disk->model[40] = '\0';
    
    kfree(identify_data);
    return 0;
}