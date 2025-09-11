#ifndef SCSI_H
#define SCSI_H

#include <stdint.h>
#include "disk.h"

/*
    SCSI Commands
*/
#define SCSI_CMD_TEST_UNIT_READY    0x00
#define SCSI_CMD_REQUEST_SENSE      0x03
#define SCSI_CMD_INQUIRY            0x12
#define SCSI_CMD_READ_CAPACITY_10   0x25
#define SCSI_CMD_READ_10            0x28
#define SCSI_CMD_WRITE_10           0x2A
#define SCSI_CMD_READ_CAPACITY_16   0x9E
#define SCSI_CMD_READ_16            0x88
#define SCSI_CMD_WRITE_16           0x8A

/*
    SCSI Command Descriptor Block
*/
typedef struct {
    uint8_t opcode;
    uint8_t flags;
    uint32_t lba;
    uint8_t group;
    uint16_t length;
    uint8_t control;
} __attribute__((packed)) scsi_cdb10_t;
#endif