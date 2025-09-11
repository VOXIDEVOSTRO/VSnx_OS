#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include "disk.h"

/*
    NVMe Controller Registers
*/
#define NVME_REG_CAP        0x00    // Controller Capabilities
#define NVME_REG_VS         0x08    // Version
#define NVME_REG_INTMS      0x0C    // Interrupt Mask Set
#define NVME_REG_INTMC      0x10    // Interrupt Mask Clear
#define NVME_REG_CC         0x14    // Controller Configuration
#define NVME_REG_CSTS       0x1C    // Controller Status
#define NVME_REG_NSSR       0x20    // NVM Subsystem Reset
#define NVME_REG_AQA        0x24    // Admin Queue Attributes
#define NVME_REG_ASQ        0x28    // Admin Submission Queue Base Address
#define NVME_REG_ACQ        0x30    // Admin Completion Queue Base Address

/*
    NVMe Command Structure
*/
typedef struct {
    uint8_t opcode;     // Command Opcode
    uint8_t flags;      // Command Flags
    uint16_t cid;       // Command Identifier
    uint32_t nsid;      // Namespace Identifier
    uint64_t rsvd;      // Reserved
    uint64_t mptr;      // Metadata Pointer
    uint64_t prp1;      // PRP Entry 1
    uint64_t prp2;      // PRP Entry 2
    uint32_t cdw10;     // Command Dword 10
    uint32_t cdw11;     // Command Dword 11
    uint32_t cdw12;     // Command Dword 12
    uint32_t cdw13;     // Command Dword 13
    uint32_t cdw14;     // Command Dword 14
    uint32_t cdw15;     // Command Dword 15
} __attribute__((packed)) nvme_cmd_t;

/*
    NVMe Completion Entry
*/
typedef struct {
    uint32_t result;    // Command-specific result
    uint32_t rsvd;      // Reserved
    uint16_t sq_head;   // Submission Queue Head Pointer
    uint16_t sq_id;     // Submission Queue Identifier
    uint16_t cid;       // Command Identifier
    uint16_t status;    // Status Field
} __attribute__((packed)) nvme_cqe_t;
#endif