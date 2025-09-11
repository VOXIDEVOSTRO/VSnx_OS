#ifndef AHCI_H
#define AHCI_H
/*
	Header files
*/
#include <stdint.h>
#include "disk.h"

/*
    AHCI Generic Host Control Registers
*/
#define AHCI_GHC_CAP        0x00    // Host Capabilities
#define AHCI_GHC_GHC        0x04    // Global Host Control
#define AHCI_GHC_IS         0x08    // Interrupt Status
#define AHCI_GHC_PI         0x0C    // Ports Implemented
#define AHCI_GHC_VS         0x10    // Version
#define AHCI_GHC_CCC_CTL    0x14    // Command Completion Coalescing Control
#define AHCI_GHC_CCC_PORTS  0x18    // Command Completion Coalescing Ports
#define AHCI_GHC_EM_LOC     0x1C    // Enclosure Management Location
#define AHCI_GHC_EM_CTL     0x20    // Enclosure Management Control
#define AHCI_GHC_CAP2       0x24    // Host Capabilities Extended
#define AHCI_GHC_BOHC       0x28    // BIOS/OS Handoff Control and Status

/*
    AHCI Port Registers (offset from port base)
*/
#define AHCI_PORT_CLB       0x00    // Command List Base Address
#define AHCI_PORT_CLBU      0x04    // Command List Base Address Upper 32-bits
#define AHCI_PORT_FB        0x08    // FIS Base Address
#define AHCI_PORT_FBU       0x0C    // FIS Base Address Upper 32-bits
#define AHCI_PORT_IS        0x10    // Interrupt Status
#define AHCI_PORT_IE        0x14    // Interrupt Enable
#define AHCI_PORT_CMD       0x18    // Command and Status
#define AHCI_PORT_TFD       0x20    // Task File Data
#define AHCI_PORT_SIG       0x24    // Signature
#define AHCI_PORT_SSTS      0x28    // Serial ATA Status
#define AHCI_PORT_SCTL      0x2C    // Serial ATA Control
#define AHCI_PORT_SERR      0x30    // Serial ATA Error
#define AHCI_PORT_SACT      0x34    // Serial ATA Active
#define AHCI_PORT_CI        0x38    // Command Issue

/*
    AHCI Command Header
*/
typedef struct {
    uint8_t cfl:5;      // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a:1;        // ATAPI
    uint8_t w:1;        // Write, 1: H2D, 0: D2H
    uint8_t p:1;        // Prefetchable
    uint8_t r:1;        // Reset
    uint8_t b:1;        // BIST
    uint8_t c:1;        // Clear busy upon R_OK
    uint8_t rsvd:1;     // Reserved
    uint8_t pmp:4;      // Port multiplier port
    uint16_t prdtl;     // Physical region descriptor table length in entries
    volatile uint32_t prdbc; // Physical region descriptor byte count transferred
    uint32_t ctba;      // Command table descriptor base address
    uint32_t ctbau;     // Command table descriptor base address upper 32 bits
    uint32_t rsv1[4];   // Reserved
} __attribute__((packed)) ahci_cmd_header_t;
#endif