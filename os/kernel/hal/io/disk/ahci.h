#ifndef AHCI_H
#define AHCI_H
/*
	header filez
	probably some complex shi here
*/
#include <stdint.h>

/*
	Register offsets
*/
#define AHCI_GHC_CAP        0x00
#define AHCI_GHC_GHC        0x04
#define AHCI_GHC_IS         0x08
#define AHCI_GHC_PI         0x0C
#define AHCI_GHC_VS         0x10

/*
	Port register offsets
*/
#define AHCI_PORT_CLB       0x00
#define AHCI_PORT_CLBU      0x04
#define AHCI_PORT_FB        0x08
#define AHCI_PORT_FBU       0x0C
#define AHCI_PORT_IS        0x10
#define AHCI_PORT_IE        0x14
#define AHCI_PORT_CMD       0x18
#define AHCI_PORT_TFD       0x20
#define AHCI_PORT_SIG       0x24
#define AHCI_PORT_SSTS      0x28
#define AHCI_PORT_SCTL      0x2C
#define AHCI_PORT_SERR      0x30
#define AHCI_PORT_SACT      0x34
#define AHCI_PORT_CI        0x38

/*
	Command header struct
*/
typedef struct {
    uint8_t cfl:5;
    uint8_t a:1;
    uint8_t w:1;
    uint8_t p:1;
    uint8_t r:1;
    uint8_t b:1;
    uint8_t c:1;
    uint8_t rsvd:1;
    uint8_t pmp:4;
    uint16_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
} __attribute__((packed)) ahci_cmd_header_t;

/*
	PRDT struct
*/
typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc:22;
    uint32_t rsv1:9;
    uint32_t i:1;
} __attribute__((packed)) ahci_prdt_entry_t;

/*
	command table struct
*/
typedef struct {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t rsv[48];
    ahci_prdt_entry_t prdt_entry[1];
} __attribute__((packed)) ahci_cmd_table_t;

/*
	some FIS
*/
#define FIS_TYPE_REG_H2D    0x27
#define FIS_TYPE_REG_D2H    0x34
#define FIS_TYPE_DMA_ACT    0x39
#define FIS_TYPE_DMA_SETUP  0x41
#define FIS_TYPE_DATA       0x46
#define FIS_TYPE_BIST       0x58
#define FIS_TYPE_PIO_SETUP  0x5F
#define FIS_TYPE_DEV_BITS   0xA1

/*
	register FIS
*/
typedef struct {
    uint8_t fis_type;
    uint8_t pmport:4;
    uint8_t rsv0:3;
    uint8_t c:1;
    uint8_t command;
    uint8_t featurel;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;
    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;
    uint8_t rsv1[4];
} __attribute__((packed)) fis_reg_h2d_t;

/*
	Port struct
*/
typedef struct {
    uint32_t base;
    uint8_t port_num;
    ahci_cmd_header_t* cmd_list;
    ahci_cmd_table_t* cmd_table;
    uint8_t* fis_base;
} ahci_port_t;

/*
	constroller sturct
*/
typedef struct {
    uint32_t abar;
    uint32_t ports_impl;
    ahci_port_t ports[32];
    uint8_t port_count;
} ahci_controller_t;

/*
	globals
*/
extern ahci_controller_t ahci_ctrl;
/*
	prototypes
*/
int ahci_init(void);
int ahci_port_init(ahci_controller_t* ctrl, uint8_t port_num);
int ahci_read(void* disk_ptr, uint64_t lba, uint32_t count, void* buffer);
int ahci_write(void* disk_ptr, uint64_t lba, uint32_t count, const void* buffer);
int ahci_identify(void* disk_ptr);
#endif